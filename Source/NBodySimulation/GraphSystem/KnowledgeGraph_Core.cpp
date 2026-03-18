// KnowledgeGraph_Core.cpp
// 
// Core simulation loop and main update functions for the force-directed graph.
// This file contains the main tick logic that drives the physics simulation,
// handles stabilization, and coordinates updates between physics and rendering.
//
// Key Functions:
// - MainFunction(): Main simulation loop called every tick
// - PostGenerateGraph(): Initialization after graph generation
// - CpuCalculateNew(): CPU physics via PhysicsSimulator (with legacy fallback)
// - UpdatePositionArray(): Dispatches to GPU or CPU physics
//
// Part of the KnowledgeGraph refactoring - extracted from KnowledgeGraph3.cpp

#include "KnowledgeGraph.h"

void AKnowledgeGraph::PostGenerateGraph()
{
	InitializeNodePosition();
	
	CalculateBiasAndStrengthOfLinks();
	
	// Initialize physics simulator
	InitializePhysicsSimulator();
	
	if (Config.bUseGPUShaders)
	{
		PassParametersToShaderManagement();
	}
	else
	{
		LogMessage("not passing parameters to shader because Config.bUseGPUShaders is false. ", true, 2);
	}
	
	LogMessage("Graph is generated. Now setting initialized to true.  ", true, 2);
	bGraphRequesting = false;
	bGraphInitialized = true;
}

void AKnowledgeGraph::UpdatePositionArray(bool log)
{
	if (Config.bUseGPUShaders)
	{
		GpuGetPositions();
	}
	else
	{
		CpuCalculateNew();
	}
}

// Legacy CpuCalculate() removed — CpuCalculateNew() in PhysicsIntegration handles
// both the PhysicsSimulator path and the fallback to ApplyForce() internally.

bool AKnowledgeGraph::MainFunction(float DeltaTime)
{
	LogMessage("MainFunction called", true, 0, TEXT("MainFunction: "));

	UpdateIterations();

	if (IsGraphStabilized(true))
	{
		if (Config.bUsePredefinedLocation)
		{
			if (bPredefinedPositionNeedsUpdate)
			{
				bPredefinedPositionNeedsUpdate = false;
				UpdateNodeWorldPositionAccordingToPositionArrayNew();
				UpdateLinkPositionNew();
			}
		}
		
		// We need to constantly run this function to draw the debug line, because it only exists for 1 frame. 
		if (Config.bUseLinkDebugLine)
		{
			UpdateLinkPositionNew();
		}
	}
	else
	{
		UpdateAlpha();
		UpdatePositionArray(true);

		UpdateNodeWorldPositionAccordingToPositionArrayNew();

		if (Config.bUpdateLinkBeforeStabilize)
		{
			LogMessage("update link position");
			UpdateLinkPositionNew();
		}
	
		if (Config.bUseGPUShaders)
		{
			UpdateParameterInShader(DeltaTime);
		}
	}
	
	if (Config.bRotateTextToFacePlayer)
	{
		RotateToFacePlayerNew();
	}
	
	return false;
}

void AKnowledgeGraph::RotateToFacePlayer()
{
	// Safety check: Ensure arrays are initialized
	if (nodePositions.Num() == 0 || GraphNodes.Num() == 0)
	{
		return;
	}

	FVector PlayerLocation = GetPlayerLocation();

	for (int i = 0; i < nodePositions.Num(); i++)
	{
		FVector NewPosition = nodePositions[i];
		if (Config.bUseTextRenderComponents)
		{
			if (Config.bRotateTextToFacePlayer && GraphNodes[i].textComponent)
			{
				// Compute the direction from the text component to the player.
				FVector ToPlayer = PlayerLocation - NewPosition;
				ToPlayer.Normalize();
				// Create a look-at rotation. The second parameter is the up-vector, adjust if needed.
				FRotator NewRotation = FRotationMatrix::MakeFromX(ToPlayer).Rotator();
				GraphNodes[i].textComponent->SetWorldRotation(NewRotation);
			}
		}
	}
}

void AKnowledgeGraph::IncreaseGraphNodeTextSize(float size)
{
	AdjustGraphNodeTextSize(true, size);
}

void AKnowledgeGraph::DecreaseGraphNodeTextSize(float size)
{
	AdjustGraphNodeTextSize(false, size);
}
