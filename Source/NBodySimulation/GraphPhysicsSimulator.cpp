// GraphPhysicsSimulator.cpp
// Implementation of physics simulation for force-directed graph layout
// Based on d3-force velocity Verlet integration algorithm
// Reference: https://github.com/d3/d3-force

#include "GraphPhysicsSimulator.h"
#include "NBodyOctree.h"
#include "KnowledgeGraph.h" // For Node77, Link77
#include "NBodyUtils.h"

void UGraphPhysicsSimulator::Initialize(const FGraphConfiguration& InConfig, const FPhysicsParameters& InParams)
{
	Config = InConfig;
	PhysicsParams = InParams;
	Iterations = 0;

	// Calculate alpha decay based on alpha min
	// Reference: d3-force (https://github.com/d3/d3-force)
	// Formula: 1 - pow(alphaMin, 1 / numIterations)
	// This creates a cooling schedule that reaches alphaMin after ~300 iterations
	// DO NOT MODIFY - based on d3-force's velocity Verlet integration
	PhysicsParams.AlphaDecay = 1.0f - FMath::Pow(PhysicsParams.AlphaMin, 1.0f / 300.0f);
	
	LogMessage("GraphPhysicsSimulator initialized");
}

/**
 * Main simulation step - applies all forces and updates node positions
 * This implements the velocity Verlet integration method from d3-force
 * 
 * Process:
 * 1. Check if simulation has stabilized (alpha < alphaMin)
 * 2. Update alpha (cooling schedule)
 * 3. Calculate and apply link forces (spring forces between connected nodes)
 * 4. Calculate and apply charge forces (repulsion between all nodes)
 * 5. Apply velocity decay (damping to prevent oscillation)
 * 6. Update positions based on velocities
 * 
 * @param DeltaTime - Time step (currently unused, alpha controls simulation speed)
 * @param NodePositions - Current positions of all nodes (modified)
 * @param NodeVelocities - Current velocities of all nodes (modified)
 * @param Nodes - Node data array
 * @param Links - Link/edge data array
 */
void UGraphPhysicsSimulator::SimulateStep(
	float DeltaTime,
	TArray<FVector>& NodePositions,
	TArray<FVector>& NodeVelocities,
	const TArray<Node77>& Nodes,
	const TArray<Link77>& Links)
{
	Iterations++;
	
	if (IsStabilized())
	{
		return; // Simulation has converged (alpha < alphaMin)
	}

	// Update alpha (simulation "temperature" - decreases over time)
	UpdateAlpha();

	// Apply link forces (spring forces between connected nodes)
	if (Config.bCalculateLinkForce)
	{
		CalculateLinkForces(NodeVelocities, NodePositions, Links);
	}

	// Apply charge forces (repulsion between all nodes)
	if (Config.bCalculateManyBodyForce)
	{
		CalculateChargeForces(NodeVelocities, NodePositions, Nodes);
	}

	// Note: Center force is currently disabled in original code
	// Center force would pull all nodes toward a central point
	// CalculateCenterForce(NodePositions, FVector::ZeroVector);

	// Apply velocity decay (damping) to prevent oscillation
	UpdateVelocitiesWithDecay(NodeVelocities);

	// Update positions using velocity Verlet integration: position += velocity
	UpdatePositionsFromVelocities(NodePositions, NodeVelocities);
}

bool UGraphPhysicsSimulator::IsStabilized() const
{
	return PhysicsParams.Alpha < PhysicsParams.AlphaMin;
}

/**
 * Calculate link forces (spring forces) between connected nodes
 * Reference: d3-force link force implementation
 * 
 * Link force acts like a spring, pulling connected nodes toward their ideal distance.
 * The force is proportional to the difference between current and ideal distance.
 * 
 * Formula: force = (distance - idealDistance) / distance * alpha * strength
 * 
 * The force is distributed between source and target nodes based on bias:
 * - bias = 0.5 means equal distribution
 * - bias closer to 1 means more force on target
 * - bias closer to 0 means more force on source
 * 
 * @param NodeVelocities - Velocities to modify (output)
 * @param NodePositions - Current node positions (input)
 * @param Links - Array of links with source, target, distance, strength, bias
 */
void UGraphPhysicsSimulator::CalculateLinkForces(
	TArray<FVector>& NodeVelocities,
	const TArray<FVector>& NodePositions,
	const TArray<Link77>& Links)
{
	for (const Link77& Link : Links)
	{
		const FVector SourcePos = NodePositions[Link.source];
		const FVector SourceVel = NodeVelocities[Link.source];
		const FVector TargetPos = NodePositions[Link.target];
		const FVector TargetVel = NodeVelocities[Link.target];

		// Calculate delta vector from source to target (including velocity)
		FVector Delta = TargetPos + TargetVel - SourcePos - SourceVel;

		// Optional: Add jiggle for zero vectors (currently disabled in original)
		// Jiggle adds small random perturbation to prevent division by zero
		// if (Delta.IsNearlyZero())
		// {
		//     Delta = Jiggle(Delta, 1e-6f);
		// }

		float Distance = Delta.Size();
		
		// Calculate spring force magnitude
		// Formula: (currentDistance - idealDistance) / currentDistance * alpha * strength
		// - If nodes are too far: positive force pulls them together
		// - If nodes are too close: negative force pushes them apart
		// - Multiplied by alpha (simulation temperature) and link strength
		float ForceMagnitude = (Distance - Link.distance * PhysicsParams.UniversalGraphScale) / Distance
			* PhysicsParams.Alpha
			* Link.strength;
		
		FVector Force = Delta * ForceMagnitude;

		// Apply force with bias distribution
		// Bias determines how force is split between source and target
		// Example: bias=0.7 means target gets 70% of force, source gets 30%
		NodeVelocities[Link.target] -= Force * Link.bias;
		NodeVelocities[Link.source] += Force * (1.0f - Link.bias);
	}
}

/**
 * Calculate charge forces (many-body repulsion) between all nodes
 * Chooses between brute force O(n²) or Barnes-Hut octree O(n log n) algorithm
 * 
 * Charge force creates repulsion between nodes, preventing them from clustering.
 * This is the most computationally expensive part of the simulation.
 * 
 * @param NodeVelocities - Velocities to modify (output)
 * @param NodePositions - Current node positions (input)
 * @param Nodes - Node data array
 */
void UGraphPhysicsSimulator::CalculateChargeForces(
	TArray<FVector>& NodeVelocities,
	const TArray<FVector>& NodePositions,
	const TArray<Node77>& Nodes)
{
	if (Config.bUseBruteForceForManyBody)
	{
		// O(n²) - check every node against every other node
		CalculateChargeForcesBruteForce(NodeVelocities, NodePositions, Nodes);
	}
	else
	{
		// O(n log n) - use Barnes-Hut octree approximation
		CalculateChargeForcesOctree(NodeVelocities, NodePositions, Nodes);
	}
}

/**
 * Calculate charge forces using Barnes-Hut octree optimization
 * Reference: Barnes & Hut (1986) - "A hierarchical O(N log N) force-calculation algorithm"
 * 
 * The Barnes-Hut algorithm reduces complexity from O(n²) to O(n log n) by:
 * 1. Building an octree spatial data structure
 * 2. Grouping distant nodes into clusters
 * 3. Approximating clusters as single bodies with center of mass
 * 
 * For each node, the algorithm traverses the octree and decides:
 * - If octree cell is far enough: treat entire cell as single body (approximation)
 * - If octree cell is too close: recurse into child cells (exact calculation)
 * 
 * The "far enough" criterion uses theta parameter (typically 0.9):
 * - cellSize / distance < theta → use approximation
 * - cellSize / distance >= theta → recurse
 * 
 * @param NodeVelocities - Velocities to modify (output)
 * @param NodePositions - Current node positions (input)
 * @param Nodes - Node data array
 */
void UGraphPhysicsSimulator::CalculateChargeForcesOctree(
	TArray<FVector>& NodeVelocities,
	const TArray<FVector>& NodePositions,
	const TArray<Node77>& Nodes)
{
	// Step 1: Build octree spatial data structure
	// Octree divides 3D space into nested cubic cells
	OctreeNode* Octree = new OctreeNode();
	Octree->AddAll1(Nodes, NodePositions);
	
	// Step 2: Calculate center of mass and total strength for each octree cell
	// This allows treating groups of nodes as single bodies
	Octree->AccumulateStrengthAndComputeCenterOfMass();

	// Step 3: Traverse octree for each node to calculate forces
	if (Config.bUseParallelProcessing)
	{
		// Parallel processing for better performance on multi-core CPUs
		ParallelFor(Nodes.Num(), [&](int32 Index)
		{
			TraverseBFS(Octree, SampleCallback, PhysicsParams.Alpha, Index, 
				const_cast<TArray<FVector>&>(NodePositions), NodeVelocities);
		});
	}
	else
	{
		// Sequential processing
		for (int32 Index = 0; Index < Nodes.Num(); Index++)
		{
			TraverseBFS(Octree, SampleCallback, PhysicsParams.Alpha, Index,
				const_cast<TArray<FVector>&>(NodePositions), NodeVelocities);
		}
	}

	// Clean up
	delete Octree;
}

/**
 * Calculate charge forces using brute force O(n²) algorithm
 * Reference: d3-force many-body force implementation
 * 
 * This method checks every node against every other node.
 * Complexity: O(n²) - slow for large graphs (>1000 nodes)
 * 
 * Charge force formula: force = direction * strength * alpha / distance²
 * - Negative strength creates repulsion (nodes push apart)
 * - Positive strength creates attraction (nodes pull together)
 * - Force decreases with square of distance (inverse square law)
 * 
 * Use Barnes-Hut octree for better performance on large graphs.
 * 
 * @param NodeVelocities - Velocities to modify (output)
 * @param NodePositions - Current node positions (input)
 * @param Nodes - Node data array
 */
void UGraphPhysicsSimulator::CalculateChargeForcesBruteForce(
	TArray<FVector>& NodeVelocities,
	const TArray<FVector>& NodePositions,
	const TArray<Node77>& Nodes)
{
	if (Config.bUseParallelProcessing)
	{
		// Parallel processing - each node calculated independently
		ParallelFor(Nodes.Num(), [&](int32 Index)
		{
			// Check this node against all other nodes
			for (int32 Index2 = 0; Index2 < Nodes.Num(); Index2++)
			{
				if (Index != Index2)
				{
					FVector Dir = NodePositions[Index2] - NodePositions[Index];
					float DistanceSq = Dir.SizeSquared();
					
					if (DistanceSq < PhysicsParams.DistanceMin)
					{
						DistanceSq = FMath::Sqrt(PhysicsParams.DistanceMin * DistanceSq);
					}
					
					NodeVelocities[Index] += Dir * PhysicsParams.NodeStrength 
						* PhysicsParams.Alpha * PhysicsParams.UniversalGraphScale / DistanceSq;
				}
			}
		});
	}
	else
	{
		// Sequential processing - simpler but slower
		for (int32 Index = 0; Index < Nodes.Num(); Index++)
		{
			// Check this node against all other nodes
			for (int32 Index2 = 0; Index2 < Nodes.Num(); Index2++)
			{
				if (Index != Index2)
				{
					// Calculate direction vector from Index to Index2
					FVector Dir = NodePositions[Index2] - NodePositions[Index];
					float DistanceSq = Dir.SizeSquared();
					
					// Prevent singularity (division by zero) when nodes are too close
					// Use geometric mean to smooth the transition
					if (DistanceSq < PhysicsParams.DistanceMin)
					{
						DistanceSq = FMath::Sqrt(PhysicsParams.DistanceMin * DistanceSq);
					}
					
					// Apply inverse square law: force = direction * strength * alpha / distance²
					// NodeStrength is typically negative (-60) for repulsion
					NodeVelocities[Index] += Dir * PhysicsParams.NodeStrength 
						* PhysicsParams.Alpha * PhysicsParams.UniversalGraphScale / DistanceSq;
				}
			}
		}
	}
}

/**
 * Calculate center force - pulls all nodes toward a central point
 * Reference: d3-force centering force
 * 
 * This force prevents the graph from drifting away by:
 * 1. Calculating the center of mass of all nodes
 * 2. Computing offset from desired center
 * 3. Shifting all nodes to center the graph
 * 
 * Note: This modifies positions directly, not velocities (unlike other forces)
 * Currently disabled in the main simulation loop.
 * 
 * @param NodePositions - Node positions to modify
 * @param Center - Desired center point (typically origin)
 */
void UGraphPhysicsSimulator::CalculateCenterForce(
	TArray<FVector>& NodePositions,
	const FVector& Center)
{
	// Calculate center of mass of all nodes
	FVector Aggregation = FVector::ZeroVector;
	for (const FVector& Pos : NodePositions)
	{
		Aggregation += Pos;
	}

	// Calculate offset from desired center
	FVector Offset = (Aggregation / NodePositions.Num() - Center);
	
	// Shift all nodes to center the graph
	for (FVector& Pos : NodePositions)
	{
		Pos -= Offset;
	}
}

void UGraphPhysicsSimulator::UpdateVelocitiesWithDecay(TArray<FVector>& NodeVelocities)
{
	// Apply velocity decay (damping) to prevent oscillation and help stabilize the simulation
	// Reference: d3-force velocity Verlet integration
	// Default decay of 0.6 means 40% of velocity is retained per tick
	// DO NOT MODIFY - based on d3-force implementation
	if (Config.bUseParallelProcessing)
	{
		ParallelFor(NodeVelocities.Num(), [&](int32 Index)
		{
			NodeVelocities[Index] *= PhysicsParams.VelocityDecay;
		});
	}
	else
	{
		for (FVector& Velocity : NodeVelocities)
		{
			Velocity *= PhysicsParams.VelocityDecay;
		}
	}
}

void UGraphPhysicsSimulator::UpdatePositionsFromVelocities(
	TArray<FVector>& NodePositions,
	const TArray<FVector>& NodeVelocities)
{
	if (Config.bUseParallelProcessing)
	{
		ParallelFor(NodePositions.Num(), [&](int32 Index)
		{
			NodePositions[Index] += NodeVelocities[Index];
		});
	}
	else
	{
		for (int32 i = 0; i < NodePositions.Num(); i++)
		{
			NodePositions[i] += NodeVelocities[i];
		}
	}
}

void UGraphPhysicsSimulator::UpdateAlpha()
{
	// Update alpha (cooling schedule)
	// Reference: d3-force alpha decay implementation
	// Alpha gradually decreases from 1.0 to alphaMin, slowing down node movement over time
	// This creates a "simulated annealing" effect for graph stabilization
	// DO NOT MODIFY - based on d3-force's velocity Verlet integration
	PhysicsParams.Alpha += (PhysicsParams.AlphaTarget - PhysicsParams.Alpha) * PhysicsParams.AlphaDecay;
}

FVector UGraphPhysicsSimulator::Jiggle(const FVector& V, float Magnitude) const
{
	FVector Result = V;
	if (FMath::IsNearlyZero(Result.X))
	{
		Result.X = (FMath::FRand() - 0.5f) * Magnitude;
	}
	if (FMath::IsNearlyZero(Result.Y))
	{
		Result.Y = (FMath::FRand() - 0.5f) * Magnitude;
	}
	if (FMath::IsNearlyZero(Result.Z))
	{
		Result.Z = (FMath::FRand() - 0.5f) * Magnitude;
	}
	return Result;
}

void UGraphPhysicsSimulator::LogMessage(const FString& Message, bool bForceLog) const
{
	if (Config.bEnableLogging || bForceLog)
	{
		UE_LOG(LogTemp, Log, TEXT("[GraphPhysicsSimulator] %s"), *Message);
	}
}
