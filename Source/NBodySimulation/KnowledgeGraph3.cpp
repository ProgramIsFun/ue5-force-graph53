#include "KnowledgeGraph.h"


// Function moved to KnowledgeGraph_Utilities.cpp

void AKnowledgeGraph::post_generate_graph()
{
	initialize_node_position();
	
	calculate_bias_and_strength_of_links();
	
	// Initialize physics simulator
	InitializePhysicsSimulator();
	
	if (use_shaders)
	{
		pass_parameters_to_shader_management();
	}else
	{
		ll("not passing parameters to shader because use_shaders is false. ", true, 2);
	}
	
	ll("Graph is generated. Now setting initialized to true.  ", true, 2);
	graph_requesting = false;
	graph_initialized = true;
}

// prepare() function moved to KnowledgeGraph_DataIntegration.cpp


// Function moved to KnowledgeGraph_Utilities.cpp

// Function moved to KnowledgeGraph_Utilities.cpp

// Text size functions moved to KnowledgeGraph_RenderIntegration.cpp


void AKnowledgeGraph::increase_text_size_of_all_nodes1112(float size)
{
	increase_or_decrease_text_size_of_all_nodes1112(true, size);
}

void AKnowledgeGraph::decrease_text_size_of_all_nodes1112(float size)
{
	increase_or_decrease_text_size_of_all_nodes1112(false, size);
}



void AKnowledgeGraph::cpu_calculate()
{
	bool log = use_logging;

	// ll("apply forces", log);
	apply_force();

	// ll("update actor location based on velocity", log);
	update_position_array_according_to_velocity_array();

}

// New version using PhysicsSimulator - defined in KnowledgeGraph_PhysicsIntegration.cpp

void AKnowledgeGraph::update_position_array(bool log)
{
	if (use_shaders)
	{
		gpu_get_positions();
		
	}else
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

// Function moved to KnowledgeGraph_Utilities.cpp

// Function moved to KnowledgeGraph_Utilities.cpp

// Function moved to KnowledgeGraph_Utilities.cpp

bool AKnowledgeGraph::main_function(float DeltaTime)
{
	bool log = use_logging;

	ll("main_function called", log, 0, TEXT("main_function: "));

	update_iterations();


	if (is_graph_stabilized(log))
	{
		if (use_predefined_location)
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
		if (link_use_debug_line)
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
		
	}else
	{

		update_alpha();
	
		// print_out_location_of_the_node();

		update_position_array(log);

		// print_out_location_of_the_node();

		// Use new renderer if available
		if (Renderer)
		{
			update_node_world_position_according_to_position_array_new();
		}
		else
		{
			update_node_world_position_according_to_position_array();
		}

		if (update_link_before_stabilize)
		{
			ll("update link position", log);
			if (Renderer)
			{
				update_link_position_new();
			}
			else
			{
				update_link_position();
			}
		}
	
		if (use_shaders){
	
			update_parameter_in_shader(DeltaTime);
		}
	}
	if (rotate_to_face_player)
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
	FVector PlayerLocation = get_player_location727();

	for (int i = 0; i < nodePositions.Num(); i++)
	{
		FVector NewPosition = nodePositions[i];
		if (node_use_text_render_components)
		{
			if (rotate_to_face_player)
			{
				// Compute the direction from the text component to the player.
				FVector ToPlayer = PlayerLocation - NewPosition;
				ToPlayer.Normalize();
				// Create a look-at rotation. The second parameter is the up-vector, adjust if needed.
				FRotator NewRotation = FRotationMatrix::MakeFromX(ToPlayer).Rotator();
				all_nodes2[i].textComponent->SetWorldRotation(NewRotation);
				// TextComponents11111111111111111111[i]->SetWorldRotation(NewRotation);
			}
		}
	}

}



// Function moved to KnowledgeGraph_Utilities.cpp

// Function moved to KnowledgeGraph_Utilities.cpp