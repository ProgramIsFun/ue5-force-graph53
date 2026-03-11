// GraphPhysicsSimulator.cpp
// Implementation of physics simulation

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
		return; // Simulation has converged
	}

	// Update alpha (simulation temperature)
	UpdateAlpha();

	// Apply forces
	if (Config.bCalculateLinkForce)
	{
		CalculateLinkForces(NodeVelocities, NodePositions, Links);
	}

	if (Config.bCalculateManyBodyForce)
	{
		CalculateChargeForces(NodeVelocities, NodePositions, Nodes);
	}

	// Note: Center force is currently disabled in original code
	// CalculateCenterForce(NodePositions, FVector::ZeroVector);

	// Update velocities with decay
	UpdateVelocitiesWithDecay(NodeVelocities);

	// Update positions based on velocities
	UpdatePositionsFromVelocities(NodePositions, NodeVelocities);
}

bool UGraphPhysicsSimulator::IsStabilized() const
{
	return PhysicsParams.Alpha < PhysicsParams.AlphaMin;
}

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

		FVector Delta = TargetPos + TargetVel - SourcePos - SourceVel;

		// Optional: Add jiggle for zero vectors (currently disabled in original)
		// if (Delta.IsNearlyZero())
		// {
		//     Delta = Jiggle(Delta, 1e-6f);
		// }

		float Distance = Delta.Size();
		
		// Calculate force magnitude
		float ForceMagnitude = (Distance - Link.distance * PhysicsParams.UniversalGraphScale) / Distance
			* PhysicsParams.Alpha
			* Link.strength;
		
		FVector Force = Delta * ForceMagnitude;

		// Apply force with bias
		NodeVelocities[Link.target] -= Force * Link.bias;
		NodeVelocities[Link.source] += Force * (1.0f - Link.bias);
	}
}

void UGraphPhysicsSimulator::CalculateChargeForces(
	TArray<FVector>& NodeVelocities,
	const TArray<FVector>& NodePositions,
	const TArray<Node77>& Nodes)
{
	if (Config.bUseBruteForceForManyBody)
	{
		CalculateChargeForcesBruteForce(NodeVelocities, NodePositions, Nodes);
	}
	else
	{
		CalculateChargeForcesOctree(NodeVelocities, NodePositions, Nodes);
	}
}

void UGraphPhysicsSimulator::CalculateChargeForcesOctree(
	TArray<FVector>& NodeVelocities,
	const TArray<FVector>& NodePositions,
	const TArray<Node77>& Nodes)
{
	// Build octree
	OctreeNode* Octree = new OctreeNode();
	Octree->AddAll1(Nodes, NodePositions);
	Octree->AccumulateStrengthAndComputeCenterOfMass();

	// Traverse octree for each node
	if (Config.bUseParallelProcessing)
	{
		ParallelFor(Nodes.Num(), [&](int32 Index)
		{
			TraverseBFS(Octree, SampleCallback, PhysicsParams.Alpha, Index, 
				const_cast<TArray<FVector>&>(NodePositions), NodeVelocities);
		});
	}
	else
	{
		for (int32 Index = 0; Index < Nodes.Num(); Index++)
		{
			TraverseBFS(Octree, SampleCallback, PhysicsParams.Alpha, Index,
				const_cast<TArray<FVector>&>(NodePositions), NodeVelocities);
		}
	}

	// Clean up
	delete Octree;
}

void UGraphPhysicsSimulator::CalculateChargeForcesBruteForce(
	TArray<FVector>& NodeVelocities,
	const TArray<FVector>& NodePositions,
	const TArray<Node77>& Nodes)
{
	if (Config.bUseParallelProcessing)
	{
		ParallelFor(Nodes.Num(), [&](int32 Index)
		{
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
		for (int32 Index = 0; Index < Nodes.Num(); Index++)
		{
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
		}
	}
}

void UGraphPhysicsSimulator::CalculateCenterForce(
	TArray<FVector>& NodePositions,
	const FVector& Center)
{
	// Calculate center of mass
	FVector Aggregation = FVector::ZeroVector;
	for (const FVector& Pos : NodePositions)
	{
		Aggregation += Pos;
	}

	// Apply centering force
	FVector Offset = (Aggregation / NodePositions.Num() - Center);
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
