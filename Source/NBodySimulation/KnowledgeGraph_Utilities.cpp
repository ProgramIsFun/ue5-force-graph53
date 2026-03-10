// KnowledgeGraph_Utilities.cpp
// Helper utility functions for KnowledgeGraph

#include "KnowledgeGraph.h"
#include "NBodyUtils.h"

FVector AKnowledgeGraph::get_player_location727()
{
	return GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
}

FVector AKnowledgeGraph::get_location_of_somewhere_in_front_of_player727()
{
	// Get the current location
	FVector CurrentLocation = get_player_location727();
	// Get the forward direction
	FVector ForwardVector = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorForwardVector();
	// Calculate the new location
	FVector NewLocation = CurrentLocation + (ForwardVector * 10000); // 100 meters away in the forward direction
	return NewLocation;
}

void AKnowledgeGraph::print_out_location_of_the_node()
{
	bool log = true;
	ll("Before update. ", log);
	ll("first element. " + nodePositions[0].ToString(), log);
	ll("second element. " + nodePositions[1].ToString(), log);
	ll("third element. " + nodePositions[2].ToString(), log);
}

void AKnowledgeGraph::update_iterations()
{
	bool log = false;
	iterations += 1;
	ll("TICK----------------------------------------------------------------------------"
	   "----------------------------------------------------------------------------", log);
	ll("iterations: " + FString::FromInt(iterations), log);
}

void AKnowledgeGraph::update_alpha()
{
	bool log = true;
	alpha += (alphaTarget - alpha) * alphaDecay; //need to restart this if want to keep moving
	ll("alpha After update, pass to the gpu later: " + FString::SanitizeFloat(alpha), log);
}

bool AKnowledgeGraph::is_graph_stabilized(bool log)
{
	// ll("alpha Before update: " + FString::SanitizeFloat(alpha), log);
	if (alpha < alphaMin)
	{
		ll("alpha is less than alphaMin", log);
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
		FNBodySimModule::Get().UpdateDeltaTime(kkkkkkkkk, alpha);
	}
}

void AKnowledgeGraph::pass_parameters_to_shader_management()
{
	SimParameters.ViewportWidth = 8000.0;
	SimParameters.CameraAspectRatio = 1.777778;
	SimParameters.GravityConstant = 1000.0;
	SimParameters.NumBodies = jnodessss;
	SimParameters.alphaS = 1;
	SimParameters.shaderdebug = static_cast<unsigned int>(use_shaders_debug);
	FNBodySimModule::Get().BeginRendering();
	FNBodySimModule::Get().InitWithParameters(SimParameters);
}

void AKnowledgeGraph::gpu_get_positions()
{
	// Retrieve GPU computed bodies position.
	TArray<FVector3f> GPUOutputPositions = FNBodySimModule::Get().GetComputedPositions();
	if (GPUOutputPositions.Num() != SimParameters.Bodies.Num())
	{
		ll("Size differ. Bodies (" +
		   FString::FromInt(SimParameters.Bodies.Num()) + ") Output(" + FString::FromInt(GPUOutputPositions.Num()) +
		   ")", true, 2);

		GPUvalid = false;
		return;
	}
	else
	{
		ll("Size is same. Bodies (" +
		   FString::FromInt(SimParameters.Bodies.Num()) + ") Output(" + FString::FromInt(GPUOutputPositions.Num()) +
		   ")", use_logging, 2);
	}

	TArray<float> alphas = FNBodySimModule::Get().GetComputedAlphas();
	ll("alpha: " + FString::SanitizeFloat(alphas[0]), use_logging, 2);
	ll("alpha1: " + FString::SanitizeFloat(alphas[1]), use_logging, 2);

	if (iterations == 1)
	{
		ll("First iteration gpu is useless!!!!!!!!!!!!!!!!!!!!!!!!! ", use_logging, 2);
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
