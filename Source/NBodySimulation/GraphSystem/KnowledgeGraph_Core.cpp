// KnowledgeGraph_Core.cpp
// 
// Core simulation loop and main update functions for the force-directed graph.
// This file contains the main tick logic that drives the physics simulation,
// handles stabilization, and coordinates updates between physics and rendering.
//
// Key Functions:
// - main_function(): Main simulation loop called every tick
// - post_generate_graph(): Initialization after graph generation
// - cpu_calculate(): Legacy CPU physics calculation (fallback)
// - update_position_array(): Dispatches to GPU or CPU physics
//
// Part of the KnowledgeGraph refactoring - extracted from KnowledgeGraph3.cpp

#include "KnowledgeGraph.h"

void AKnowledgeGraph::post_generate_graph()
{
	initialize_node_position();
	
	calculate_bias_and_strength_of_links();
	
	// Initialize physics simulator
	InitializePhysicsSimulator();
	
	if (Config.bUseGPUShaders)
	{
		pass_parameters_to_shader_management();
	}
	else
	{
		LogMessage("not passing parameters to shader because Config.bUseGPUShaders is false. ", true, 2);
	}
	
	LogMessage("Graph is generated. Now setting initialized to true.  ", true, 2);
	graph_requesting = false;
	graph_initialized = true;
}

void AKnowledgeGraph::update_position_array(bool log)
{
	if (Config.bUseGPUShaders)
	{
		gpu_get_positions();
	}
	else
	{
		// Use new physics simulator if available, otherwise fall back to old method
		if (PhysicsSimulator)
		{
			cpu_calculate_new();
		}
		else
		{
			cpu_calculate();
		}
	}
}

void AKnowledgeGraph::cpu_calculate()
{
	bool log = Config.bEnableLogging;

	// Safety check: Ensure arrays are initialized before simulation
	if (nodePositions.Num() == 0 || nodeVelocities.Num() == 0 || GraphNodes.Num() == 0)
	{
		LogMessage("Arrays not initialized yet, skipping cpu_calculate", log, 1);
		return;
	}

	apply_force();
	update_position_array_according_to_velocity_array();
}

bool AKnowledgeGraph::main_function(float DeltaTime)
{
	bool log = Config.bEnableLogging;

	LogMessage("main_function called", log, 0, TEXT("main_function: "));

	update_iterations();

	if (is_graph_stabilized(log))
	{
		if (Config.bUsePredefinedLocation)
		{
			if (use_predefined_position_should_update_once)
			{
				use_predefined_position_should_update_once = false;
				// Use new renderer if available
				if (Renderer)
				{
					update_node_world_position_according_to_position_array_new();
					update_link_position_new();
				}
				else
				{
					update_node_world_position_according_to_position_array();
					update_link_position();
				}
			}
		}
		
		// We need to constantly run this function to draw the debug line, because it only exists for 1 frame. 
		if (Config.bUseLinkDebugLine)
		{
			if (Renderer)
			{
				update_link_position_new();
			}
			else
			{
				update_link_position();
			}
		}
	}
	else
	{
		update_alpha();
		update_position_array(log);

		// Use new renderer if available
		if (Renderer)
		{
			update_node_world_position_according_to_position_array_new();
		}
		else
		{
			update_node_world_position_according_to_position_array();
		}

		if (Config.bUpdateLinkBeforeStabilize)
		{
			LogMessage("update link position", log);
			if (Renderer)
			{
				update_link_position_new();
			}
			else
			{
				update_link_position();
			}
		}
	
		if (Config.bUseGPUShaders)
		{
			update_parameter_in_shader(DeltaTime);
		}
	}
	
	if (Config.bRotateTextToFacePlayer)
	{
		if (Renderer)
		{
			rotate_to_face_player_new();
		}
		else
		{
			rotate_to_face_player111();
		}
	}
	
	return false;
}

void AKnowledgeGraph::rotate_to_face_player111()
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
			if (Config.bRotateTextToFacePlayer)
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

void AKnowledgeGraph::increase_text_size_of_all_nodes1112(float size)
{
	increase_or_decrease_text_size_of_all_nodes1112(true, size);
}

void AKnowledgeGraph::decrease_text_size_of_all_nodes1112(float size)
{
	increase_or_decrease_text_size_of_all_nodes1112(false, size);
}
