// KnowledgeGraph_Core.cpp
// 
// Core simulation loop and main update functions for the force-directed graph.
// This file contains the main tick logic that drives the physics simulation,
// handles stabilization, and coordinates updates between physics and rendering.
//
// Key Functions:
// - MainFunction(): Main simulation loop called every tick
// - PostGenerateGraph(): Initialization after graph generation
// - CpuCalculate(): Legacy CPU physics calculation (fallback)
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
		// Use new physics simulator if available, otherwise fall back to old method
		if (PhysicsSimulator)
		{
			CpuCalculateNew();
		}
		else
		{
			CpuCalculate();
		}
	}
}

void AKnowledgeGraph::CpuCalculate()
{
	bool log = Config.bEnableLogging;

	// Safety check: Ensure arrays are initialized before simulation
	if (nodePositions.Num() == 0 || nodeVelocities.Num() == 0 || GraphNodes.Num() == 0)
	{
		LogMessage("Arrays not initialized yet, skipping CpuCalculate", log, 1);
		return;
	}

	ApplyForce();
	UpdatePositionArrayAccordingToVelocityArray();
}

bool AKnowledgeGraph::MainFunction(float DeltaTime)
{
	bool log = Config.bEnableLogging;

	LogMessage("MainFunction called", log, 0, TEXT("MainFunction: "));

	UpdateIterations();

	if (IsGraphStabilized(log))
	{
		if (Config.bUsePredefinedLocation)
		{
			if (bPredefinedPositionNeedsUpdate)
			{
				bPredefinedPositionNeedsUpdate = false;
				// Use new renderer if available
				if (Renderer)
				{
					UpdateNodeWorldPositionAccordingToPositionArrayNew();
					UpdateLinkPositionNew();
				}
				else
				{
					UpdateNodeWorldPositionAccordingToPositionArray();
					UpdateLinkPosition();
				}
			}
		}
		
		// We need to constantly run this function to draw the debug line, because it only exists for 1 frame. 
		if (Config.bUseLinkDebugLine)
		{
			if (Renderer)
			{
				UpdateLinkPositionNew();
			}
			else
			{
				UpdateLinkPosition();
			}
		}
	}
	else
	{
		UpdateAlpha();
		UpdatePositionArray(log);

		// Use new renderer if available
		if (Renderer)
		{
			UpdateNodeWorldPositionAccordingToPositionArrayNew();
		}
		else
		{
			UpdateNodeWorldPositionAccordingToPositionArray();
		}

		if (Config.bUpdateLinkBeforeStabilize)
		{
			LogMessage("update link position", log);
			if (Renderer)
			{
				UpdateLinkPositionNew();
			}
			else
			{
				UpdateLinkPosition();
			}
		}
	
		if (Config.bUseGPUShaders)
		{
			UpdateParameterInShader(DeltaTime);
		}
	}
	
	if (Config.bRotateTextToFacePlayer)
	{
		if (Renderer)
		{
			RotateToFacePlayerNew();
		}
		else
		{
			RotateToFacePlayer();
		}
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
