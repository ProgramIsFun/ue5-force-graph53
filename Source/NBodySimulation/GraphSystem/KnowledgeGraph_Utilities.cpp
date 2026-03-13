// KnowledgeGraph_Utilities.cpp
//
// Helper utility functions for the KnowledgeGraph system.
// Contains commonly used helper functions for player location, logging,
// shader management, and GPU position retrieval.
//
// Key Functions:
// - GetPlayerLocation(): Gets current player position
// - update_alpha(): Updates simulation cooling parameter
// - is_graph_stabilized(): Checks if simulation has converged
// - gpu_get_positions(): Retrieves positions from GPU compute shader
//
// Part of the KnowledgeGraph refactoring - extracted from KnowledgeGraph2/3.cpp

#include "KnowledgeGraph.h"
#include "NBodyUtils.h"

FVector AKnowledgeGraph::GetPlayerLocation()
{
	return GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
}

FVector AKnowledgeGraph::GetLocationInFrontOfPlayer()
{
	// Get the current location
	FVector CurrentLocation = GetPlayerLocation();
	// Get the forward direction
	FVector ForwardVector = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorForwardVector();
	// Calculate the new location
	FVector NewLocation = CurrentLocation + (ForwardVector * 10000); // 100 meters away in the forward direction
	return NewLocation;
}

void AKnowledgeGraph::print_out_location_of_the_node()
{
	bool log = true;
	LogMessage("Before update. ", log);
	LogMessage("first element. " + nodePositions[0].ToString(), log);
	LogMessage("second element. " + nodePositions[1].ToString(), log);
	LogMessage("third element. " + nodePositions[2].ToString(), log);
}

void AKnowledgeGraph::update_iterations()
{
	bool log = false;
	iterations += 1;
	LogMessage("TICK----------------------------------------------------------------------------"
	   "----------------------------------------------------------------------------", log);
	LogMessage("iterations: " + FString::FromInt(iterations), log);
}

void AKnowledgeGraph::update_alpha()
{
	bool log = true;
	Config.Alpha += (Config.AlphaTarget - Config.Alpha) * Config.AlphaDecay; //need to restart this if want to keep moving
	LogMessage("alpha After update, pass to the gpu later: " + FString::SanitizeFloat(Config.Alpha), log);
}

bool AKnowledgeGraph::is_graph_stabilized(bool log)
{
	// LogMessage("alpha Before update: " + FString::SanitizeFloat(Config.Alpha), log);
	if (Config.Alpha < Config.AlphaMin)
	{
		LogMessage("alpha is less than alphaMin", log);
		FNBodySimModule::Get().EndRendering();
		// update_link_position();
		return true;
	}
	return false;
}

void AKnowledgeGraph::update_parameter_in_shader(float DeltaTime)
{
	if (1)
	{
		float kkkkkkkkk = 1;
		SimParameters.DeltaTime = kkkkkkkkk;
		FNBodySimModule::Get().UpdateDeltaTime(kkkkkkkkk, Config.Alpha);
	}
}

void AKnowledgeGraph::pass_parameters_to_shader_management()
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

void AKnowledgeGraph::gpu_get_positions()
{
	// Retrieve GPU computed bodies position.
	TArray<FVector3f> GPUOutputPositions = FNBodySimModule::Get().GetComputedPositions();
	if (GPUOutputPositions.Num() != SimParameters.Bodies.Num())
	{
		LogMessage("Size differ. Bodies (" +
		   FString::FromInt(SimParameters.Bodies.Num()) + ") Output(" + FString::FromInt(GPUOutputPositions.Num()) +
		   ")", true, 2);

		GPUvalid = false;
		return;
	}
	else
	{
		LogMessage("Size is same. Bodies (" +
		   FString::FromInt(SimParameters.Bodies.Num()) + ") Output(" + FString::FromInt(GPUOutputPositions.Num()) +
		   ")", Config.bEnableLogging, 2);
	}

	TArray<float> alphas = FNBodySimModule::Get().GetComputedAlphas();
	LogMessage("alpha: " + FString::SanitizeFloat(alphas[0]), Config.bEnableLogging, 2);
	LogMessage("alpha1: " + FString::SanitizeFloat(alphas[1]), Config.bEnableLogging, 2);

	if (iterations == 1)
	{
		LogMessage("First iteration gpu is useless!!!!!!!!!!!!!!!!!!!!!!!!! ", Config.bEnableLogging, 2);
		GPUvalid = false;
		return;
	}

	for (int i = 0; i < SimParameters.Bodies.Num(); i++)
	{
		FVector NewPosition = FVector(GPUOutputPositions[i]);
		nodePositions[i] = NewPosition;
	}
	GPUvalid = true;
}

void AKnowledgeGraph::debug_test()
{
	// Debug test function - currently unused
}
