#include "KnowledgeGraph.h"


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

void AKnowledgeGraph::post_generate_graph()
{
	initialize_node_position();
	
	calculate_bias_and_strength_of_links();
	
	if (use_shaders)
	{
		pass_parameters_to_shader_management();
	}
	
	graph_requesting = false;
	graph_initialized = true;
}

void AKnowledgeGraph::prepare()
{
	request_a_graph();
	
	if(use_predefined_location)
	{

		// Predefine locations could be regarded stable  
		alpha = 0;
	}
}


void AKnowledgeGraph::update_parameter_in_shader(float DeltaTime)
{
	if (1)
	{
		float kkkkkkkkk=1;
		SimParameters.DeltaTime = kkkkkkkkk;
		FNBodySimModule::Get().UpdateDeltaTime(kkkkkkkkk, alpha);
	}
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

void AKnowledgeGraph::set_text_size_of_all_nodes1112(float size)
{
	for (int i = 0; i < jnodessss; i++)
	{
		if (node_use_text_render_components)
		{
			TextComponents11111111111111111111[i]->SetWorldSize(size);
		}
	} 
}
void AKnowledgeGraph::increase_or_decrease_text_size_of_all_nodes1112(bool increase, float size)
{
	for (int i = 0; i < jnodessss; i++)
	{
		if (node_use_text_render_components)
		{
			float current_size = TextComponents11111111111111111111[i]->WorldSize;
			if (increase)
			{
				current_size += size;
			}else
			{
				current_size -= size;
			}
			TextComponents11111111111111111111[i]->SetWorldSize(current_size);
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



void AKnowledgeGraph::cpu_calculate()
{
	bool log = use_logging;

	// ll("apply forces", log);
	apply_force();

	// ll("update actor location based on velocity", log);
	update_position_array_according_to_velocity_array();

}

void AKnowledgeGraph::update_position_array(bool log)
{
	if (use_shaders)
	{
		gpu_get_positions();
		
	}else
	{
		cpu_calculate();
	}
}

void AKnowledgeGraph::update_alpha()
{
	bool log=true;
	alpha += (alphaTarget - alpha) * alphaDecay; //need to restart this if want to keep moving
	ll("alpha After update, pass to the gpu later: " + FString::SanitizeFloat(alpha), log);
}

void AKnowledgeGraph::print_out_location_of_the_node()
{
	bool log=true;
	ll("Before update. ", log);
	ll("first element. " + nodePositions[0].ToString(), log);
	ll("second element. " + nodePositions[1].ToString(), log);
	ll("third element. " + nodePositions[2].ToString(), log);
}

void AKnowledgeGraph::update_iterations()
{
	bool log=false;
	iterations += 1;
	ll("TICK----------------------------------------------------------------------------"
	   "----------------------------------------------------------------------------", log);
	ll("iterations: " + FString::FromInt(iterations), log);
}

bool AKnowledgeGraph::main_function(float DeltaTime)
{
	bool log = use_logging;

	
	update_iterations();


	if (is_graph_stabilized(log))
	{
		if (use_predefined_location)
		{
			if (use_predefined_position_should_update_once)
			{
				use_predefined_position_should_update_once = false;
				update_node_world_position_according_to_position_array();
				update_link_position();
			}
			
		}
		
		// We need to constantly run this function to draw the debug line, because it only exists for 1 frame. 
		if (link_use_debug_line)
		{
			update_link_position();
		}
		
	}else
	{

		update_alpha();
	
		// print_out_location_of_the_node();

		update_position_array(log);

		// print_out_location_of_the_node();

		update_node_world_position_according_to_position_array();

		if (update_link_before_stabilize)
		{
			ll("update link position", log);
			update_link_position();
		}
	
		if (use_shaders){
	
			update_parameter_in_shader(DeltaTime);
		}
	}
	
	
	
	return false;
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
	}else
	{
		ll("Size is same. Bodies (" +
		   FString::FromInt(SimParameters.Bodies.Num()) + ") Output(" + FString::FromInt(GPUOutputPositions.Num()) +
		   ")", use_logging, 2);
	}

	TArray<float> alphas = FNBodySimModule::Get().GetComputedAlphas();
	ll("alpha: " + FString::SanitizeFloat(alphas[0]), use_logging, 2);
	ll("alpha1: " + FString::SanitizeFloat(alphas[1]), use_logging, 2);

	// ll("First element position is: " + GPUOutputPositions[0].ToString(), use_logging, 2);
	// ll("second element position is: " + GPUOutputPositions[1].ToString(), use_logging, 2);
	// ll("third element position is: " + GPUOutputPositions[2].ToString(), use_logging, 2);
	//
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
	if (0)
	{
	}
	else
	{
		TArray<UStaticMeshComponent*> CylinderMeshes;

		// Loop to create 10 Cylinder Meshes
		for (int32 i = 0; i < 10; ++i)
		{
			UStaticMeshComponent* NewCylinderMesh = NewObject<UStaticMeshComponent>(this, FName(*FString::Printf(TEXT("CylinderMesh%d"), i)));
			NewCylinderMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			NewCylinderMesh->RegisterComponent();

			// Set scale
			NewCylinderMesh->SetWorldScale3D(FVector(10, 1, 1));
    
			// Set static mesh
			NewCylinderMesh->SetStaticMesh(link_use_static_meshlinkMesh);
    
			// Set different relative locations for each cylinder to avoid overlap
			NewCylinderMesh->SetRelativeLocation(FVector(10, 10, 100 + i * 120));

			// Add to the array
			CylinderMeshes.Add(NewCylinderMesh);
		}
	}
}