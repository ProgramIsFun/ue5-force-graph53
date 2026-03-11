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

	// Build physics parameters from Config
	FPhysicsParameters PhysicsParams;
	PhysicsParams.Alpha = Config.Alpha;
	PhysicsParams.AlphaMin = Config.AlphaMin;
	PhysicsParams.AlphaTarget = Config.AlphaTarget;
	PhysicsParams.AlphaDecay = Config.AlphaDecay;
	PhysicsParams.EdgeDistance = Config.EdgeDistance;
	PhysicsParams.NodeStrength = Config.NodeStrength;
	PhysicsParams.VelocityDecay = Config.VelocityDecay;
	PhysicsParams.DistanceMin = Config.DistanceMin;
	PhysicsParams.DistanceMax = Config.DistanceMax;
	PhysicsParams.InitialRadius = Config.InitialRadius;
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

	// Safety check: Ensure arrays are initialized before simulation
	if (nodePositions.Num() == 0 || nodeVelocities.Num() == 0 || GraphNodes.Num() == 0)
	{
		LogMessage("Arrays not initialized yet (nodePositions: " + FString::FromInt(nodePositions.Num()) + 
		          ", nodeVelocities: " + FString::FromInt(nodeVelocities.Num()) + 
		          ", GraphNodes: " + FString::FromInt(GraphNodes.Num()) + "), skipping physics step", true, 1);
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

	// Sync alpha back to Config (for compatibility and UI display)
	Config.Alpha = PhysicsSimulator->GetAlpha();
}
