#include "KnowledgeGraph.h"

#include <random>


#include "utillllllssss.h"
#include <map>
#include "GameFramework/Character.h"


#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::White,text)


bool AKnowledgeGraph::generate_actor_and_register(AKnowledgeNode*& kn)
{
	kn = GetWorld()->SpawnActor<AKnowledgeNode>();

	if (kn)
	{
		UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(kn);
		MeshComp->AttachToComponent(
			kn->GetRootComponent(),
			FAttachmentTransformRules::SnapToTargetIncludingScale
		);
		MeshComp->RegisterComponent(); // Don't forget to register the component


		float sss = node_use_actor_size;
		FVector NewScale = FVector(sss, sss, sss);
		MeshComp->SetWorldScale3D(NewScale);


		UStaticMesh* CubeMesh;
		if (false)
		{
			CubeMesh = LoadObject<UStaticMesh>(
				nullptr,
				TEXT(
					"/Engine/BasicShapes/Cube.Cube"
				)
			);
		}
		CubeMesh = SelectedMesh1111111111111;
		if (CubeMesh)
		{
			MeshComp->SetStaticMesh(CubeMesh);
		}
		else
		{
			ll("CubeMesh failed", true, 2);
			qq();
			return true;
		}
	}
	return false;
}

void AKnowledgeGraph::generate_text_render_component_and_attach(FString name,int32 index)
{
	UTextRenderComponent* TextComponent = NewObject<UTextRenderComponent>(
		this, FName("TextComponent" + name)
	);
	if (TextComponent)
	{
		TextComponent->SetText(FText::FromString(name));
		TextComponent->SetupAttachment(RootComponent);
		TextComponent->SetWorldSize(text_size);
		TextComponent->RegisterComponent(); // This is important to initialize the component
		all_nodes2[index].textComponent = TextComponent;
		// TextComponents11111111111111111111.Add(TextComponent);
	}
}

void AKnowledgeGraph::get_number_of_nodes()
{
	if (cgm == CGM::GENERATE)

	{
		jnodessss = jnodes1;
	}
	if (cgm == CGM::JSON || cgm == CGM::DATABASE)
	{
		TArray<TSharedPtr<FJsonValue>> jnodes = JsonObject1->GetArrayField("nodes");
		jnodessss = jnodes.Num();
	}
}

void AKnowledgeGraph::create_one_to_one_mapping()
{
	bool log = false;


	TArray<TSharedPtr<FJsonValue>> jnodes = JsonObject1->GetArrayField("nodes");
	for (int32 i = 0; i < jnodessss; i++)
	{
		auto jobj = jnodes[i]->AsObject();
		FString jid;

		if (cgm == CGM::JSON)
		{
			jid = jobj->GetStringField("id");
		}
		if (cgm == CGM::DATABASE)
		{
			jid = jobj->GetStringField("user_generate_id_7577777777");
		}


		ll("jid: " + jid, log);
		string_to_id.Emplace(jid, i);
		id_to_string.Emplace(i, jid);
	}
}

void AKnowledgeGraph::miscellaneous()
{
	bool log = false;
	// Edge creation loop
	if (!connect_to_previous)
	{
		for (int32 i = 1; i < jnodessss; i++)
		{
			int jid = i - 1;
			int jsource = i; // Ensures jsource is always valid within the index range

			// Connected to random node 
			int jtarget = FMath::RandRange(0, i - 1);
			add_edge(jid, jsource, jtarget);
		}
	}
	else
	{
		ll("Randomly connected is disabled    will always connect to the previous node. ", log);
		for (int32 i = 1; i < jnodessss; i++)
		{
			int jid = i - 1;
			int jsource = i; // Ensures jsource is always valid within the index range

			// Connected to random node 
			int jtarget = i - 1;
			add_edge(jid, jsource, jtarget);
		}
	}
}

void AKnowledgeGraph::set_array_lengths()
{
	nodePositions.SetNumUninitialized(jnodessss);
	nodeVelocities.SetNumUninitialized(jnodessss);
	all_nodes2.SetNumUninitialized(jnodessss);
	
	if (use_shaders)
	{
		SimParameters.Bodies.SetNumUninitialized(
			jnodessss
		);
	}
	if (node_use_instance_static_mesh)
	{
		BodyTransforms.SetNumUninitialized(
			jnodessss);
	}
}

void AKnowledgeGraph::set_array_values()
{
	for (FVector& velocity : nodeVelocities)
	{
		velocity.X = 0.0f;
		velocity.Y = 0.0f;
		velocity.Z = 0.0f;
	}
}

void AKnowledgeGraph::initialize_arrays()
{
	
	set_array_lengths();

	set_array_values();

		
}

bool AKnowledgeGraph::generate_objects_for_node_and_link()
{
	bool log = false;
	if (cgm == CGM::GENERATE)
	{
		for (int32 i = 0; i < jnodessss; i++)
		{
			if (node_use_text_render_components)
			{
				FString name;
				name = "Sample Text : " + FString::FromInt(i);
				generate_text_render_component_and_attach(name,i);
			}
		}
		miscellaneous();
	}
	else
	{
		TArray<TSharedPtr<FJsonValue>> jnodes = JsonObject1->GetArrayField("nodes");
		for (int32 i = 0; i < jnodessss; i++)
		{
			auto jobj = jnodes[i]->AsObject();
			if (node_use_text_render_components)
			{
				FString name;
				try
				{
					name = jobj->GetStringField("name");

					FString Substring(TEXT("everythingallaccount"));
					FString ReplacementSubstring(TEXT("e"));

					if (name.StartsWith(Substring))
					{
						// Remove the substring by creating a new string that starts right after the substring
						name = name.Mid(Substring.Len());

						name = ReplacementSubstring + name;
					}
				}
				catch (...)
				{
					name = "Sample Text : " + FString::FromInt(i);
				}
				generate_text_render_component_and_attach(name,i);
			}
		}

		TArray<TSharedPtr<FJsonValue>> jedges = JsonObject1->GetArrayField("links");
		ll("jedges.Num(): " + FString::FromInt(jedges.Num()), log);
		
		for (int32 i = 0; i < jedges.Num(); i++)
		{
			auto jobj = jedges[i]->AsObject();
			FString jid;
			FString jsourceS = jobj->GetStringField("source");
			FString jtargetS = jobj->GetStringField("target");
			int jsource = string_to_id[jsourceS];
			int jtarget = string_to_id[jtargetS];
			ll("jsource: " + FString::FromInt(jsource) + ", jtarget: " + FString::FromInt(jtarget), log);
			add_edge(i, jsource, jtarget);
		}
	}
	return false;
}

void AKnowledgeGraph::extracting_property_list_and_store()
{
	// Assuming 'JsonObject1' is already a valid TSharedPtr<FJsonObject> pointing to a parsed JSON object
	TArray<TSharedPtr<FJsonValue>> jnodes = JsonObject1->GetArrayField("nodes");

	// Placeholder for all nodes properties
	// TArray<TMap<FString, FString>> AllNodeProperties;

	for (int32 i = 0; i < jnodes.Num(); i++)
	{
		TSharedPtr<FJsonObject> jobj = jnodes[i]->AsObject();
		if (jobj.IsValid())
		{
			// Create a map for this node's properties
			TMap<FString, FString> NodeProperties;

			// Iterate through all key-value pairs in the JSON object
			for (const auto& Elem : jobj->Values)
			{
				// Elem.Key is the FString key, Elem.Value is the TSharedPtr<FJsonValue>
				// Ensure that the value can be represented as a string
				if (Elem.Value.IsValid() && Elem.Value->Type == EJson::String)
				{
					FString ValueAsString = Elem.Value->AsString();
					NodeProperties.Add(Elem.Key, ValueAsString);
				}
				// As shown here, handle other types similarly if needed
				else if (Elem.Value.IsValid() && Elem.Value->Type == EJson::Number)
				{
					NodeProperties.Add(Elem.Key, FString::SanitizeFloat(Elem.Value->AsNumber()));
				}
				// Add more conditions as needed for types like Bool, Arrays, etc.
			}

			// Add the populated map to the array of all nodes' properties
			AllNodeProperties.Add(NodeProperties);
		}
	}
}

void AKnowledgeGraph::deal_with_predefined_location()
{
	bool log=false;
	predefined_positions.SetNumUninitialized(jnodessss);

	if (cgm == CGM::DATABASE)
	{
		// Retrieve the position of the nodes from the database
		// and set the position of the nodes to the retrieved position.
		// This is done by setting the nodePositions array to the retrieved position
		TArray<TSharedPtr<FJsonValue>> jnodes = JsonObject1->GetArrayField("nodes");
		for (int32 i = 0; i < jnodessss; i++)
		{
			auto jobj = jnodes[i]->AsObject();
			FVector jlocation;

			FString jid = jobj->GetStringField("user_generate_id_7577777777");


			if (jobj->HasField("ue_location_X") &&
				jobj->HasField("ue_location_Y") &&
				jobj->HasField("ue_location_Z")
			)
			{
				jlocation = FVector(
					jobj->GetNumberField("ue_location_X"),
					jobj->GetNumberField("ue_location_Y"),
					jobj->GetNumberField("ue_location_Z")
				);
				// You can use jlocation vector as needed
			}
			else
			{
				// Send a warning to the client. 
				ll("location does not exist", log);

				// Handle cases where location coordinates do not exist
				// For example, assigning a default value or logging an error
				jlocation = FVector(0, 0, 0); // Default value if no location found
			}


			ll("location111111111111111: " + jlocation.ToString(), log);

			// int id111 = string_to_id[jid];
			// predefined_positions[id111] = jlocation;
			predefined_positions[i] = jlocation;
		}


		if (use_predefined_locationand_then_center_to_current_actor)
		{
			FVector center = GetActorLocation();
			FVector aggregation = FVector(0, 0, 0);

			for (int32 i = 0; i < jnodessss; i++)
			{
				aggregation += predefined_positions[i];
			}

			aggregation /= jnodessss;
			for (int32 i = 0; i < jnodessss; i++)
			{
				predefined_positions[i] -= (aggregation - center);
			}
		}
	}
	else
	{
		ll("Pretty fine location feature is only available for using database.  ", log);
		qq();
	}
}

void AKnowledgeGraph::default_generate_graph_method()
{
	bool log = true;

	get_number_of_nodes();

	if (
		cgm == CGM::JSON || cgm == CGM::DATABASE
	)
	{
		create_one_to_one_mapping();
	}


	initialize_arrays();


	if (
		cgm == CGM::DATABASE
	)
	{
		extracting_property_list_and_store();
	}
	
	if (generate_objects_for_node_and_link())
	{
		return;
	}


	if (use_predefined_location)
	{	
		deal_with_predefined_location();
	}


	post_generate_graph();
}


void AKnowledgeGraph::calculate_link_force_and_update_velocity()
{
	bool log = true;


	// link forces
	// After loop, the velocity of all notes have been altered a little bit because of the link force already. 
	int32 Index = 0;
	for (auto& link : all_links2)
	{
		ll("link iteration: !!!!!!!!!!!!!!!!!!" + FString::FromInt(Index), log);

		FVector source_pos = nodePositions[link.source];
		FVector source_velocity = nodeVelocities[link.source];
		FVector target_pos = nodePositions[link.target];
		FVector target_velocity = nodeVelocities[link.target];

		FVector new_v = target_pos + target_velocity - source_pos - source_velocity;


		ll("new_v: " + new_v.ToString(), log);
		ll("target_pos- source_pos: " + (target_pos - source_pos).ToString(), log);
		if (false)
		{
			if (new_v.IsNearlyZero())
			{
				new_v = Jiggle(new_v, 1e-6f);
			}
			ll("GIGGLE is enabled............", log);
		}
		ll("GIGGLE is disabled............"
		   "...................................................  "
		   ""
		   "Remember to turn it back on. ", log);

		float l = new_v.Size();

		// ll("l: " + FString::SanitizeFloat(l), log);
		// By looking at the javascript code, we can see strength Will only be computed when there is a change Of the graph structure to the graph.
		l = (l - link.distance * universal_graph_scale) /
			l
			* alpha
			* link.strength;
		new_v *= l;

		ll("before update nodeVelocities", log);
		ll("nodeVelocities[" + FString::FromInt(link.target) + "]: " + nodeVelocities[link.target].ToString(), log);
		ll("nodeVelocities[" + FString::FromInt(link.source) + "]: " + nodeVelocities[link.source].ToString(), log);
		nodeVelocities[link.target] -= new_v * (link.bias);
		nodeVelocities[link.source] += new_v * (1 - link.bias);

		ll("after update nodeVelocities", log);
		ll("nodeVelocities[" + FString::FromInt(link.target) + "]: " + nodeVelocities[link.target].ToString(), log);
		ll("nodeVelocities[" + FString::FromInt(link.source) + "]: " + nodeVelocities[link.source].ToString(), log);

		Index++;
	}
}

void AKnowledgeGraph::calculate_charge_force_and_update_velocity()
{
	bool log = true;
	bool log2 = false;


	if (!cpu_many_body_use_brute_force)
	{
		//
		OctreeData2 = new OctreeNode(
		);


		OctreeData2->AddAll1(
			all_nodes2,
			nodePositions);

		OctreeData2->AccumulateStrengthAndComputeCenterOfMass();

		// lll("tttttttttttttttttttttttt");
		ll("!!!OctreeData2->CenterOfMass: " + OctreeData2->CenterOfMass.ToString(), log);
		ll("!!!OctreeData2->strength: " + FString::SanitizeFloat(OctreeData2->Strength), log);


		if (!cpu_use_parallel)
		{
			int32 Index = 0;
			for (auto& node : all_nodes2)
			{
				ll("--------------------------------------", log);
				ll(
					"Traverse the tree And calculate velocity on this Actor Kn, nodekey: -"
					+
					FString::FromInt(
						Index
					), log);


				TraverseBFS(OctreeData2,
				            SampleCallback,
				            alpha,
				            Index
				            ,
				            nodePositions,
				            nodeVelocities
				);
				ll("Finished traversing the tree based on this Actor Kn. ", log);
				Index++;
			}
		}
		else
		{
			ParallelFor(all_nodes2.Num(), [&](int32 Index)
			{
				TraverseBFS(OctreeData2,
				            SampleCallback, alpha, Index, nodePositions, nodeVelocities);
			});
		}


		ll("Finished traversing, now we can delete the tree. ", log);
		delete OctreeData2;
	}
	else
	{
		if (!cpu_use_parallel)
		{
			// Brute force
			int32 Index = 0;
			for (auto& node : all_nodes2)
			{
				auto kn = node.node;


				int32 Index2 = 0;

				for (auto& node2 : all_nodes2)
				{
					auto kn2 = node2.node;
					if (kn != kn2)
					{
						// FVector dir = kn2->GetActorLocation() - kn->GetActorLocation();
						FVector dir = nodePositions[
							Index2] - nodePositions[Index];

						float l = dir.Size() * dir.Size();
						if (l < distancemin)
						{
							l = sqrt(distancemin * l);
						}
						nodeVelocities[Index] += dir * nodeStrength * alpha / l;
						// kn->velocity += dir * nodeStrength * alpha / l; 
					}
					Index2++;
				}


				Index++;
			}
		}
		else
		{
			ParallelFor(all_nodes2.Num(), [&](int32 Index)
			{
				auto node = all_nodes2[Index];


				int32 Index2 = 0;
				for (auto& node2 : all_nodes2)
				{
					auto kn2 = node2.node;
					if (node.node != kn2)
					{
						// FVector dir = kn2->GetActorLocation() - kn->GetActorLocation();
						FVector dir = nodePositions[Index2] - nodePositions[Index];

						float l = dir.Size() * dir.Size();
						if (l < distancemin)
						{
							l = sqrt(distancemin * l);
						}
						nodeVelocities[Index] += dir * nodeStrength * alpha * universal_graph_scale / l;
						// kn->velocity += dir * nodeStrength * alpha / l; 
					}
					Index2++;
				}
			});
		}
	}
}

void AKnowledgeGraph::calculate_centre_force_and_update_position()
{
	// Following is javascript implementation of Center Force
	// for (i = 0; i < n; ++i) {
	// 	node = nodes[i],
	// 		sx += node.x || 0,
	// 		sy += node.y || 0,
	// 		sz += node.z || 0;
	// }
	//
	// for (sx = (sx / n - x) * strength,
	// 		 sy = (sy / n - y) * strength,
	// 		 sz = (sz / n - z) * strength,
	// 		 i = 0;
	// 	 i < n;
	// 	 ++i
	// ) {
	// 	node = nodes[i];
	// 	if (sx) {
	// 		node.x -= sx
	// 	}
	// 	if (sy) {
	// 		node.y -= sy;
	// 	}
	// 	if (sz) {
	// 		node.z -= sz;
	// 	}
	// }
	FVector center = FVector(0, 0, 0);
	FVector aggregation = FVector(0, 0, 0);


	int32 Index = 0;
	for (auto& node : all_nodes2)
	{
		// aggregation += node.Value->GetActorLocation();
		aggregation += nodePositions[
			Index
		];

		Index++;
	}

	Index = 0;
	for (auto& node : all_nodes2)
	{
		nodePositions[
				Index
			] =
			nodePositions[
				Index
			] - (
				aggregation / all_nodes2.Num() - center
			) * 1;
		// node.Value->SetActorLocation(
		// 	node.Value->GetActorLocation() - (aggregation / all_nodes.Num() - center) * 1
		// );
		Index++;
	}


	ll("Ignoring the update position step for now in the center force. ");
}


void AKnowledgeGraph::update_position_array_according_to_velocity_array()
{
	if (!cpu_use_parallel)
	{
		int32 Index = 0;
		for (auto& node : all_nodes2)
		{
			auto kn = node.node;

			nodeVelocities[
				Index
			] *= velocityDecay;
			// kn->velocity *= velocityDecay;

			// FVector NewLocation = kn->GetActorLocation() + kn->velocity;

			// 	kn->SetActorLocation(
			// 	NewLocation
			// );

			nodePositions[
				Index
			] = nodePositions[
				Index

			] + nodeVelocities[
				Index

			];

			Index++;
		}
	}
	else
	{
		// Assertion failed: ComponentsThatNeedEndOfFrameUpdate_OnGameThread.IsValidIndex(ArrayIndex) [File:D:\build\++UE5\Sync\Engine\Source\Runtime\Engine\Private\LevelTick.cpp] [Line: 872]

		ParallelFor(all_nodes2.Num(), [&](int32 Index)
		{
			nodeVelocities[Index] *= velocityDecay;
			nodePositions[Index] = nodePositions[Index] + nodeVelocities[Index];
		});
	}
}

void AKnowledgeGraph::update_link_position()
{
	for (auto& link : all_links2)
	{
		FVector Location1 = nodePositions[link.source];
		FVector Location2 = nodePositions[link.target];


		if (link_use_static_mesh)
		{
			auto l = link.edgeMesh;

			FVector MidPoint = (Location1 + Location2) / 2.0f;
			FVector ForwardVector = (Location2 - Location1);
			float CylinderHeight = ForwardVector.Size();

			FRotator Rotation = FRotationMatrix::MakeFromZ(ForwardVector).Rotator();

			l->SetWorldLocation(
				Location1
			);

			l->SetWorldScale3D(
				FVector(
					link_use_static_mesh_thickness,
					link_use_static_mesh_thickness,
					link_use_static_mesh_length_fine_tune * CylinderHeight)
			);
			l->SetWorldRotation(
				Rotation
			);
		}
		if (link_use_debug_line)
		{
			UWorld* World = GetWorld();
			if (!World)
			{
				return;
			}

			DrawDebugLine(
				World,
				Location1,
				Location2,
				FColor::Red,
				false,
				-1,
				0,
				10.0f
			);
		}
	}
}

void AKnowledgeGraph::apply_force()
{
	bool log = true;

	// In here velocity of all notes are zeroed
	// In the following for loop, In the first few loop, the velocity is 0. 


	if (cpu_linkc)
	{
		ll("Ready to calculate link.--------------------------------------", log);
		calculate_link_force_and_update_velocity();
		ll("Finish calculating link.--------------------------------------", log);
	}
	else
	{
		ll("cpu_linkc is disabled. ", log);
	}


	if (cpu_manybody)
	{
		ll("Ready to calculate charge.--------------------------------------", log);

		calculate_charge_force_and_update_velocity();
		ll("Finish calculating charge.--------------------------------------", log);
	}
	else
	{
		ll("cpu_manybody is disabled. ", log);
	}


	if (true)
	{
		ll("centre force is disabled for debugging. ", log);
	}
	else
	{
		calculate_centre_force_and_update_position();
	}
}


void AKnowledgeGraph::initialize_node_position()
{
	if (initialize_using_actor_location)
	{
		current_own_position = GetActorLocation();
	}

	if (!cpu_use_parallel)
	{
		for (
			int32 index = 0; index < jnodessss; index++
		)
		{
			initialize_node_position_individual(
				index);
		}
	}
	else
	{
		ParallelFor(
			jnodessss, [&](int32 index)
			{
				initialize_node_position_individual(
					index);
			}
		);
	}

	if (node_use_instance_static_mesh)
	{
		InstancedStaticMeshComponent->AddInstances(BodyTransforms, false);
	}
}

void AKnowledgeGraph::initialize_node_position_individual(int index)
{
	FVector init_pos;

	if (use_predefined_location)
	{
		init_pos = predefined_positions[index];
	}
	else
	{
		// Calculate index-based radius
		float radius;
		int nDim = 3;
		if (nDim > 2)
		{
			radius = initialRadius * cbrt(0.5f + index);
		}
		else if (nDim > 1)
		{
			radius = initialRadius * sqrt(0.5f + index);
		}
		else
		{
			radius = initialRadius * index;
		}

		float initialAngleRoll = PI * (3 - sqrt(5)); // Roll angle

		// Following will be Math.PI * 20 / (9 + Math.sqrt(221));
		float initialAngleYaw = PI * 20 / (9 + sqrt(221)); // Yaw angle if needed (3D)


		float rollAngle = index * initialAngleRoll; // Roll angle
		float yawAngle = index * initialAngleYaw; // Yaw angle if needed (3D)


		if (nDim == 1)
		{
			// 1D: Positions along X axis
			init_pos = FVector(
				radius * universal_graph_scale,
				0,
				0);
		}
		else if (nDim == 2)
		{
			// 2D: Circular distribution
			init_pos = FVector(
				radius * cos(rollAngle) * universal_graph_scale,
				radius * sin(rollAngle) * universal_graph_scale,
				0
			);
		}
		else
		{
			// 3D: Spherical distribution
			init_pos = FVector(
				radius * sin(rollAngle) * cos(yawAngle) * universal_graph_scale,
				radius * cos(rollAngle) * universal_graph_scale,
				radius * sin(rollAngle) * sin(yawAngle) * universal_graph_scale
			);
		}

		if (initialize_using_actor_location)
		{
			init_pos += current_own_position;
		}
	}

	nodePositions[index] = init_pos;

	ll("index: " + FString::FromInt(index) + " init_pos: " + init_pos.ToString());

	if (node_use_instance_static_mesh)
	{
		float s = node_use_instance_static_mesh_size;
		FTransform MeshTransform(
			FRotator(),
			init_pos,
			FVector(s, s, s)
		);
		BodyTransforms[index] = MeshTransform;
	}

	if (use_shaders)
	{
		FVector3f RandomVelocity
		{
			0, 0, 0
		};
		float RandomMass = FMath::FRandRange(
			20.0
			,
			50.0);
		SimParameters.Bodies[index] = FBodyData(
			RandomMass,
			FVector3f(init_pos),
			RandomVelocity);
	}
}

FVector AKnowledgeGraph::get_player_location727()
{
	return GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
}

void AKnowledgeGraph::update_node_world_position_according_to_position_array()
{
	if (use_shaders && !GPUvalid)
	{
		return;
	}

	FVector PlayerLocation = get_player_location727();

	// Update bodies visual with new positions.
	for (int i = 0; i < nodePositions.Num(); i++)
	{
		FVector NewPosition = nodePositions[i];

		
		if (node_use_instance_static_mesh)
		{
			BodyTransforms[i].SetTranslation(NewPosition);
		}

		if (node_use_text_render_components)
		{
			all_nodes2[i].textComponent->SetWorldLocation(NewPosition);
			// TextComponents11111111111111111111[i]->SetWorldLocation(NewPosition);

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

	if (node_use_instance_static_mesh)
	{
		InstancedStaticMeshComponent->BatchUpdateInstancesTransforms(0, BodyTransforms, false, true);
	}
}

void AKnowledgeGraph::calculate_bias_and_strength_of_links()
{
	bool log = false;
	float n = all_nodes2.Num();
	float m = all_links2.Num();

	std::map<int32, int32> Nodeconnection;

	std::map<int, std::vector<int>> connectout;
	std::map<int, std::vector<int>> connectin;


	if (use_shaders)
	{
		int m2 = m * 2;
		SimParameters.NumLinks = m2;

		LinkOffsets.SetNumUninitialized(n);
		LinkCounts.SetNumUninitialized(n);
		LinkIndices.SetNumUninitialized(m2);
		LinkStrengths.SetNumUninitialized(m2); // Holds the strength of each link
		LinkBiases.SetNumUninitialized(m2); // Holds the bias of each link
		Linkinout.SetNumUninitialized(m2);
	}

	for (auto& link : all_links2)
	{
		Nodeconnection[link.source] += 1;
		Nodeconnection[link.target] += 1;

		if (use_shaders)
		{
			connectout[link.source].push_back(link.target);
			connectin[link.target].push_back(link.source);
		}
	}


	if (!use_shaders)
	{
		int i = 0;
		for (auto& link : all_links2)
		{
			int s1 = Nodeconnection[link.source];
			int s2 = Nodeconnection[link.target];

			float ttttttttttt = s1 + s2;
			float bias = s1 / ttttttttttt;

			link.bias = bias;

			// Better have a check because strength is set to 1 when it is initialized. 
			link.strength = 1.0 / fmin(s1,
			                           s2);
			ll("i: " + FString::FromInt(i), log);
			ll("link.bias: " + FString::SanitizeFloat(link.bias), log);
			ll("link.strength: " + FString::SanitizeFloat(link.strength), log);
			i++;
		}
	}
	else
	{
		int32 Index = 0;
		for (int i = 0; i < n; i++)
		{
			ll("i: " + FString::FromInt(i), log);

			int outcount = connectout[i].size();
			int incount = connectin[i].size();

			ll("outcount: " + FString::FromInt(outcount), log);
			ll("incount: " + FString::FromInt(incount), log);

			int totalcount = Nodeconnection[i];

			if (totalcount != outcount + incount)
			{
				ll("totalcount!=outcount+incount", true, 2);
				qq();
			}


			LinkOffsets[i] = Index;

			ll("LinkOffsets[i]: " + FString::FromInt(LinkOffsets[i]), log);

			LinkCounts[i] = Nodeconnection[i];
			ll("LinkCounts[i]: " + FString::FromInt(LinkCounts[i]), log);

			for (int j = 0; j < outcount; j++)
			{
				int counterpart = connectout[i][j];

				int indexnow = Index + j;

				LinkIndices[
					indexnow
				] = counterpart;
				Linkinout[
					indexnow
				] = 1;

				int s1 = Nodeconnection[i];
				int s2 = Nodeconnection[connectout[i][j]];

				float ttttttttttt = s1 + s2;
				float bias = s1 / ttttttttttt;
				LinkBiases[indexnow] = bias;
				LinkStrengths[indexnow] = 1.0 / fmin(s1,
				                                     s2);
			}
			for (int j = 0; j < incount; j++)
			{
				int counterpart = connectin[i][j];
				int indexnow = Index + outcount + j;

				LinkIndices[indexnow] = counterpart;

				Linkinout[indexnow] = 0;

				int s2 = Nodeconnection[i];
				int s1 = Nodeconnection[counterpart];

				float ttttttttttt = s1 + s2;
				float bias = s1 / ttttttttttt;
				LinkBiases[indexnow] = bias;
				LinkStrengths[indexnow] = 1.0 / fmin(s1,
				                                     s2);
			}
			Index += Nodeconnection[i];
		}


		FString ConcatenatedString;
		for (int32 Number : LinkOffsets)
		{
			ConcatenatedString += FString::Printf(TEXT("%d "), Number);
		}
		ll("LinkOffsets: " + ConcatenatedString, log);

		ConcatenatedString = "";
		for (int32 Number : LinkCounts)
		{
			ConcatenatedString += FString::Printf(TEXT("%d "), Number);
		}
		ll("LinkCounts: " + ConcatenatedString, log);

		ConcatenatedString = "";
		for (int32 Number : LinkIndices)
		{
			ConcatenatedString += FString::Printf(TEXT("%d "), Number);
		}
		ll("LinkIndices: " + ConcatenatedString, log);

		ConcatenatedString = "";
		for (float Number : LinkStrengths)
		{
			ConcatenatedString += FString::Printf(TEXT("%f "), Number);
		}
		ll("LinkStrengths: " + ConcatenatedString, log);

		ConcatenatedString = "";
		for (float Number : LinkBiases)
		{
			ConcatenatedString += FString::Printf(TEXT("%f "), Number);
		}
		ll("LinkBiases: " + ConcatenatedString, log);

		ConcatenatedString = "";
		for (int32 Number : Linkinout)
		{
			ConcatenatedString += FString::Printf(TEXT("%d "), Number);
		}
		ll("Linkinout: " + ConcatenatedString, log);


		SimParameters.LinkOffsets = LinkOffsets;
		SimParameters.LinkCounts = LinkCounts;
		SimParameters.LinkIndices = LinkIndices;
		SimParameters.LinkStrengths = LinkStrengths;
		SimParameters.LinkBiases = LinkBiases;
		SimParameters.Linkinout = Linkinout;
	}
}


bool AKnowledgeGraph::generate_actor_for_a_link(Link& link)
{
	AKnowledgeEdge* e;
	UClass* bpClass;
	if (true)
	{
		// This approach works in both play and editor and package game. 
		UClass* loadedClass = StaticLoadClass(UObject::StaticClass(), nullptr,
		                                      TEXT(
			                                      // "Blueprint'/Game/Characters/Enemies/BP_LitchBoss1.BP_LitchBoss1_C'"
			                                      "Blueprint'/Game/arttttttt/iii9.iii9_C'"
		                                      ));
		if (loadedClass)
		{
			e = GetWorld()->SpawnActor<AKnowledgeEdge>(loadedClass);
		}
		else
		{
			qq();
			return true;
			ll("error loading classsssssssssssssssssssssss");
			e = GetWorld()->SpawnActor<AKnowledgeEdge>(
				AKnowledgeEdge::StaticClass()
			);
		}
	}
	else
	{
		// This approach works in Only play in editor
		// Load the Blueprint
		UBlueprint* LoadedBP = Cast<UBlueprint>(StaticLoadObject(
				UBlueprint::StaticClass(),
				nullptr,
				TEXT(
					// "Blueprint'/Game/NewBlueprint22222.NewBlueprint22222'"
					"Blueprint'/Game/kkkkk/NewBlueprint22222.NewBlueprint22222'"
				)
			)
		);
		if (!LoadedBP)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load the Blueprint."));
			eeeee();
		}
		// Check if the Blueprint class is valid
		bpClass = LoadedBP->GeneratedClass;
		if (!bpClass)
		{
			UE_LOG(LogTemp, Error, TEXT("Generated class from Blueprint is null."));
			eeeee();
		}
		e = GetWorld()->SpawnActor<AKnowledgeEdge>(
			bpClass
		);
	}
	link.edge = e;
	return false;
}

void AKnowledgeGraph::add_edge(int32 id, int32 source, int32 target)
{
	
	Link link = Link(source, target);
	


	if (link_use_static_mesh)
	{
		UStaticMeshComponent* CylinderMesh;
		// Dynamically create the mesh component and attach it
		CylinderMesh = NewObject<UStaticMeshComponent>(this,

		                                               FName(*FString::Printf(TEXT("CylinderMesh%d"), id))

		);

		CylinderMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		CylinderMesh->RegisterComponent(); // Registers the component with the World so it gets rendered and updated

		CylinderMesh->SetWorldScale3D(FVector(1, 1, 1));

		CylinderMesh->SetStaticMesh(
			link_use_static_mesh_mesh
		);

		CylinderMesh->SetMaterial(0, CylinderMaterial);

		
		link.edgeMesh = CylinderMesh;
	}

	link.strength = 1;
	link.distance = edgeDistance;

	all_links2.Add(link);
}
