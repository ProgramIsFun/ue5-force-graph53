// KnowledgeGraph_PhysicsIntegration.cpp
// Integration layer between KnowledgeGraph and GraphPhysicsSimulator

#include "KnowledgeGraph.h"
#include "NBodyUtils.h"

// Initialize physics simulator with current settings
void AKnowledgeGraph::InitializePhysicsSimulator()
{
	if (!PhysicsSimulator)
	{
		LogMessage("PhysicsSimulator is null!", true, 2);
		return;
	}

	// Build physics parameters from current settings
	FPhysicsParameters PhysicsParams;
	PhysicsParams.Alpha = alpha;
	PhysicsParams.AlphaMin = alphaMin;
	PhysicsParams.AlphaTarget = alphaTarget;
	PhysicsParams.AlphaDecay = alphaDecay;
	PhysicsParams.EdgeDistance = edgeDistance;
	PhysicsParams.NodeStrength = nodeStrength;
	PhysicsParams.VelocityDecay = velocityDecay;
	PhysicsParams.DistanceMin = distancemin;
	PhysicsParams.DistanceMax = distancemax;
	PhysicsParams.InitialRadius = initialRadius;
	PhysicsParams.UniversalGraphScale = Config.UniversalGraphScale;

	PhysicsSimulator->Initialize(Config, PhysicsParams);
	
	LogMessage("PhysicsSimulator initialized", true, 0);
}

// New CPU calculate using PhysicsSimulator
void AKnowledgeGraph::cpu_calculate_new()
{
	if (!PhysicsSimulator)
	{
		LogMessage("PhysicsSimulator is null, falling back to old method", true, 1);
		cpu_calculate();
		return;
	}

	// Run one simulation step
	PhysicsSimulator->SimulateStep(
		GetWorld()->GetDeltaSeconds(),
		nodePositions,
		nodeVelocities,
		GraphNodes,
		GraphLinks
	);

	// Sync alpha back to main class (for compatibility)
	alpha = PhysicsSimulator->GetAlpha();
}
