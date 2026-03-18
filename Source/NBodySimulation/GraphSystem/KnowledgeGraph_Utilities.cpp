// KnowledgeGraph_Utilities.cpp
//
// Helper utility functions for the KnowledgeGraph system.
// Contains commonly used helper functions for player location, logging,
// shader management, and GPU position retrieval.
//
// Key Functions:
// - GetPlayerLocation(): Gets current player position
// - UpdateAlpha(): Updates simulation cooling parameter
// - IsGraphStabilized(): Checks if simulation has converged
// - GpuGetPositions(): Retrieves positions from GPU compute shader
//
// Part of the KnowledgeGraph refactoring - extracted from KnowledgeGraph2/3.cpp

#include "KnowledgeGraph.h"
#include "NBodyUtils.h"

FVector AKnowledgeGraph::GetPlayerLocation()
{
	UWorld* World = GetWorld();
	if (!World) { return FVector::ZeroVector; }
	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerController && PlayerController->GetPawn())
	{
		return PlayerController->GetPawn()->GetActorLocation();
	}
	LogToScreen("WARNING: No player controller or pawn found, returning zero vector", true, 2);
	return FVector::ZeroVector;
}

FVector AKnowledgeGraph::GetLocationInFrontOfPlayer()
{
	FVector CurrentLocation = GetPlayerLocation();
	UWorld* World = GetWorld();
	if (!World) { return CurrentLocation; }
	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerController && PlayerController->GetPawn())
	{
		FVector ForwardVector = PlayerController->GetPawn()->GetActorForwardVector();
		// 10000 units = 100 meters in UE scale
		constexpr float GraphSpawnDistanceFromPlayer = 10000.0f;
		return CurrentLocation + (ForwardVector * GraphSpawnDistanceFromPlayer);
	}
	LogToScreen("WARNING: No player controller or pawn found for forward vector", true, 2);
	return CurrentLocation;
}

void AKnowledgeGraph::UpdateIterations()
{
	bool log = false;
	SimulationIterationCount += 1;
	LogMessage("TICK----------------------------------------------------------------------------"
	   "----------------------------------------------------------------------------", log);
	LogMessage("iterations: " + FString::FromInt(SimulationIterationCount), log);
}

void AKnowledgeGraph::UpdateAlpha()
{
	bool log = true;
	Config.Alpha += (Config.AlphaTarget - Config.Alpha) * Config.AlphaDecay; //need to restart this if want to keep moving
	LogMessage("alpha After update, pass to the gpu later: " + FString::SanitizeFloat(Config.Alpha), log);
}

bool AKnowledgeGraph::IsGraphStabilized(bool log)
{
	// LogMessage("alpha Before update: " + FString::SanitizeFloat(Config.Alpha), log);
	if (Config.Alpha < Config.AlphaMin)
	{
		LogMessage("alpha is less than alphaMin", log);
		FNBodySimModule::Get().EndRendering();
		// UpdateLinkPosition();
		return true;
	}
	return false;
}

void AKnowledgeGraph::UpdateParameterInShader(float DeltaTime)
{
	float ShaderDeltaTime = 1.0f;
	SimParameters.DeltaTime = ShaderDeltaTime;
	FNBodySimModule::Get().UpdateDeltaTime(ShaderDeltaTime, Config.Alpha);
}

void AKnowledgeGraph::PassParametersToShaderManagement()
{
	SimParameters.ViewportWidth = 8000.0;
	SimParameters.CameraAspectRatio = 1.777778;
	SimParameters.GravityConstant = 1000.0;
	SimParameters.NumBodies = TotalNodeCount;
	SimParameters.alphaS = 1;
	SimParameters.shaderdebug = static_cast<unsigned int>(Config.ShaderDebugLevel);
	FNBodySimModule::Get().BeginRendering();
	FNBodySimModule::Get().InitWithParameters(SimParameters);
}

void AKnowledgeGraph::GpuGetPositions()
{
	// Retrieve GPU computed bodies position.
	TArray<FVector3f> GPUOutputPositions = FNBodySimModule::Get().GetComputedPositions();
	if (GPUOutputPositions.Num() != SimParameters.Bodies.Num())
	{
		LogMessage("Size differ. Bodies (" +
		   FString::FromInt(SimParameters.Bodies.Num()) + ") Output(" + FString::FromInt(GPUOutputPositions.Num()) +
		   ")", true, 2);

		bGPUResultValid = false;
		return;
	}
	else
	{
		LogMessage("Size is same. Bodies (" +
		   FString::FromInt(SimParameters.Bodies.Num()) + ") Output(" + FString::FromInt(GPUOutputPositions.Num()) +
		   ")", Config.bEnableLogging, 2);
	}

	TArray<float> alphas = FNBodySimModule::Get().GetComputedAlphas();
	LogMessage("alpha: " + FString::SanitizeFloat(alphas[0]), true, 2);
	LogMessage("alpha1: " + FString::SanitizeFloat(alphas[1]), true, 2);

	if (SimulationIterationCount == 1)
	{
		LogMessage("First iteration gpu is useless!!!!!!!!!!!!!!!!!!!!!!!!! ", true, 2);
		bGPUResultValid = false;
		return;
	}

	for (int i = 0; i < SimParameters.Bodies.Num(); i++)
	{
		FVector NewPosition = FVector(GPUOutputPositions[i]);
		nodePositions[i] = NewPosition;
	}
	bGPUResultValid = true;
}

// GraphNode and GraphLink are plain C++ structs (not USTRUCT) holding UObject pointers
// (UTextRenderComponent*, UStaticMeshComponent*) that the GC cannot see through reflection.
// We use AddReferencedObjects instead of converting to USTRUCT+UPROPERTY because this graph
// can have thousands of nodes/links — a direct C++ loop is faster than the reflection-based
// property walk that USTRUCT would require every GC cycle.
void AKnowledgeGraph::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	AKnowledgeGraph* This = CastChecked<AKnowledgeGraph>(InThis);

	// Register UTextRenderComponent pointers inside GraphNodes with the GC
	for (GraphNode& Node : This->GraphNodes)
	{
		if (Node.textComponent)
		{
			Collector.AddReferencedObject(Node.textComponent);
		}
	}

	// Register UStaticMeshComponent pointers inside GraphLinks with the GC
	for (GraphLink& Link : This->GraphLinks)
	{
		if (Link.EdgeMeshComponent)
		{
			Collector.AddReferencedObject(Link.EdgeMeshComponent);
		}
	}

	Super::AddReferencedObjects(InThis, Collector);
}
