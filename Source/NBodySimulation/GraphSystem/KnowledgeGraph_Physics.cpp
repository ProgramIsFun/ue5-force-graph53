#include "KnowledgeGraph.h"

#include <random>


#include "NBodyUtils.h"
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


		float sss = Config.NodeActorSize;
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
		CubeMesh = Config.NodeMesh;
		if (CubeMesh)
		{
			MeshComp->SetStaticMesh(CubeMesh);
		}
		else
		{
			LogMessage("CubeMesh failed", true, 2);
			QuitGame();
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
		TextComponent->SetWorldSize(Config.TextSize);
		TextComponent->RegisterComponent(); // This is important to initialize the component
		GraphNodes[index].textComponent = TextComponent;
		// TextComponents11111111111111111111.Add(TextComponent);
	}
}

void AKnowledgeGraph::get_number_of_nodes()
{
	if (Config.CreationMode == EGraphCreationMode::AutoGenerate)
	{
		LogMessage("Generating graph automatically. Number of nodes: " + FString::FromInt(Config.AutoGenerateNodeCount), true, 0,
		   TEXT("get_number_of_nodes: "));
		TotalNodeCount = Config.AutoGenerateNodeCount;
	}
	if (Config.CreationMode == EGraphCreationMode::FromJson || Config.CreationMode == EGraphCreationMode::FromDatabase)
	{
		if (!JsonObject1.IsValid())
		{
			LogMessage("ERROR: JsonObject1 is invalid!", true, 3);
			TotalNodeCount = 0;
			precheck_succeed = false;
			return;
		}
		
		TArray<TSharedPtr<FJsonValue>> jnodes = JsonObject1->GetArrayField("nodes");
		TotalNodeCount = jnodes.Num();
		
		// Safety check for reasonable node count
		if (TotalNodeCount < 0 || TotalNodeCount > 100000)
		{
			LogMessage("ERROR: Invalid node count: " + FString::FromInt(TotalNodeCount), true, 3);
			TotalNodeCount = 0;
			precheck_succeed = false;
			return;
		}
		
		LogMessage("Loaded node count from JSON: " + FString::FromInt(TotalNodeCount), true, 0);
	}
}

void AKnowledgeGraph::create_one_to_one_mapping()
{
	// Create one-to-one mapping between string IDs and integer indices
	
	bool log = false;
	
	TArray<TSharedPtr<FJsonValue>> jnodes = JsonObject1->GetArrayField("nodes");
	for (int32 i = 0; i < TotalNodeCount; i++)
	{
		auto jobj = jnodes[i]->AsObject();
		FString jid;

		if (Config.CreationMode == EGraphCreationMode::FromJson)
		{
			jid = jobj->GetStringField("id");
		}
		if (Config.CreationMode == EGraphCreationMode::FromDatabase)
		{
			jid = jobj->GetStringField("user_generate_id_7577777777");
		}


		LogMessage("jid: " + jid, log);
		string_to_id.Emplace(jid, i);
		id_to_string.Emplace(i, jid);
	}
}

void AKnowledgeGraph::miscellaneous()
{
	GraphLinks.SetNumUninitialized(TotalNodeCount-1);

	bool log = false;
	// Edge creation loop
	if (!Config.bConnectToAdjacentNodeOnly)
	{
		for (int32 i = 1; i < TotalNodeCount; i++)
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
		LogMessage("Randomly connected is disabled    will always connect to the previous node. ", log);
		for (int32 i = 1; i < TotalNodeCount; i++)
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
	// Safety check to prevent memory allocation crashes
	if (TotalNodeCount <= 0 || TotalNodeCount > 100000)
	{
		LogMessage("ERROR: Invalid TotalNodeCount value: " + FString::FromInt(TotalNodeCount) + ". Refusing to allocate arrays.", true, 3);
		precheck_succeed = false;
		return;
	}
	
	LogMessage("Setting array lengths for " + FString::FromInt(TotalNodeCount) + " nodes", true, 0);
	
	nodePositions.SetNumUninitialized(TotalNodeCount);
	nodeVelocities.SetNumUninitialized(TotalNodeCount);
	GraphNodes.SetNumUninitialized(TotalNodeCount);
	
	if (Config.bUseGPUShaders)
	{
		SimParameters.Bodies.SetNumUninitialized(TotalNodeCount);
	}
	if (Config.bUseInstancedStaticMesh)
	{
		BodyTransforms.SetNumUninitialized(TotalNodeCount);
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
	bool log = true;
	if (Config.CreationMode == EGraphCreationMode::AutoGenerate)
	{
		for (int32 i = 0; i < TotalNodeCount; i++)
		{
			if (Config.bUseTextRenderComponents)
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
		for (int32 i = 0; i < TotalNodeCount; i++)
		{
			auto jobj = jnodes[i]->AsObject();
			if (Config.bUseTextRenderComponents)
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
		LogMessage("Number of node generated: " + FString::FromInt(TotalNodeCount), log);







		TArray<TSharedPtr<FJsonValue>> jedges = JsonObject1->GetArrayField("links");
		LogMessage("jedges.Num(): " + FString::FromInt(jedges.Num()), log);
		GraphLinks.SetNumUninitialized(jedges.Num());


		for (int32 i = 0; i < jedges.Num(); i++)
		{
			auto jobj = jedges[i]->AsObject();
			FString jid;
			FString jsourceS = jobj->GetStringField("source");
			FString jtargetS = jobj->GetStringField("target");
			int jsource = string_to_id[jsourceS];
			int jtarget = string_to_id[jtargetS];
			// LogMessage("jsource: " + FString::FromInt(jsource) + ", jtarget: " + FString::FromInt(jtarget), log);
			add_edge(i, jsource, jtarget);
		}

		LogMessage("Number of link generated: " + FString::FromInt(jedges.Num()), log);
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
	bool log=Config.bEnableLogging;
	predefined_positions.SetNumUninitialized(TotalNodeCount);

	if (Config.CreationMode == EGraphCreationMode::FromDatabase)
	{
		// Retrieve the position of the nodes from the database
		// and set the position of the nodes to the retrieved position.
		// This is done by setting the nodePositions array to the retrieved position
		TArray<TSharedPtr<FJsonValue>> jnodes = JsonObject1->GetArrayField("nodes");
		for (int32 i = 0; i < TotalNodeCount; i++)
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
				LogMessage("location does not exist", log);

				// Handle cases where location coordinates do not exist
				// For example, assigning a default value or logging an error
				jlocation = FVector(0, 0, 0); // Default value if no location found
			}


			LogMessage("location111111111111111: " + jlocation.ToString(), log);

			// int id111 = string_to_id[jid];
			// predefined_positions[id111] = jlocation;
			predefined_positions[i] = jlocation;
		}


		if (Config.bCenterPredefinedLocationToActor)
		{
			FVector center = GetActorLocation();
			FVector aggregation = FVector(0, 0, 0);

			for (int32 i = 0; i < TotalNodeCount; i++)
			{
				aggregation += predefined_positions[i];
			}

			aggregation /= TotalNodeCount;
			for (int32 i = 0; i < TotalNodeCount; i++)
			{
				predefined_positions[i] -= (aggregation - center);
			}
		}
	}
	else
	{
		LogMessage("predefined_location location feature is only available for using database.  ", log);
	}
}

void AKnowledgeGraph::default_generate_graph_method()
{

	get_number_of_nodes();

	if (
		Config.CreationMode == EGraphCreationMode::FromJson || Config.CreationMode == EGraphCreationMode::FromDatabase
	)
	{
		LogMessage("creating one to one mapping", true, 0, TEXT("default_generate_graph_method: "));
		create_one_to_one_mapping();
	}else
	{
		LogMessage("auto generate graph, no need to create one to one mapping", true, 0, TEXT("default_generate_graph_method: "));
	}
	
	initialize_arrays();
	
	if (Config.CreationMode == EGraphCreationMode::FromDatabase)
	{
		extracting_property_list_and_store();
	}else
	{
		
	}
	
	if (generate_objects_for_node_and_link())
	{
		return;
	}

	if (Config.bUsePredefinedLocation)
	{	
		deal_with_predefined_location();
	}else
	{
		LogMessage("not using predefined location", true, 0, TEXT("default_generate_graph_method: "));
	}

	LogMessage("post generate graph", true, 0, TEXT("default_generate_graph_method: "));
	post_generate_graph();
}


void AKnowledgeGraph::calculate_link_force_and_update_velocity()
{
	bool log = true;


	// link forces
	// After loop, the velocity of all notes have been altered a little bit because of the link force already. 
	int32 Index = 0;
	for (auto& link : GraphLinks)
	{
		LogMessage("link iteration: !!!!!!!!!!!!!!!!!!" + FString::FromInt(Index), log);

		FVector source_pos = nodePositions[link.SourceNodeIndex];
		FVector source_velocity = nodeVelocities[link.SourceNodeIndex];
		FVector target_pos = nodePositions[link.target];
		FVector target_velocity = nodeVelocities[link.target];

		FVector new_v = target_pos + target_velocity - source_pos - source_velocity;


		LogMessage("new_v: " + new_v.ToString(), log);
		LogMessage("target_pos- source_pos: " + (target_pos - source_pos).ToString(), log);
		if (false)
		{
			if (new_v.IsNearlyZero())
			{
				new_v = Jiggle(new_v, 1e-6f);
			}
			LogMessage("GIGGLE is enabled............", log);
		}
		LogMessage("GIGGLE is disabled............"
		   "...................................................  "
		   ""
		   "Remember to turn it back on. ", log);

		float l = new_v.Size();

		// LogMessage("l: " + FString::SanitizeFloat(l), log);
		// By looking at the javascript code, we can see strength Will only be computed when there is a change Of the graph structure to the graph.
		l = (l - link.distance * Config.UniversalGraphScale) /
			l
			* Config.Alpha
			* link.strength;
		new_v *= l;

		LogMessage("before update nodeVelocities", log);
		LogMessage("nodeVelocities[" + FString::FromInt(link.target) + "]: " + nodeVelocities[link.target].ToString(), log);
		LogMessage("nodeVelocities[" + FString::FromInt(link.SourceNodeIndex) + "]: " + nodeVelocities[link.SourceNodeIndex].ToString(), log);
		nodeVelocities[link.target] -= new_v * (link.bias);
		nodeVelocities[link.SourceNodeIndex] += new_v * (1 - link.bias);

		LogMessage("after update nodeVelocities", log);
		LogMessage("nodeVelocities[" + FString::FromInt(link.target) + "]: " + nodeVelocities[link.target].ToString(), log);
		LogMessage("nodeVelocities[" + FString::FromInt(link.SourceNodeIndex) + "]: " + nodeVelocities[link.SourceNodeIndex].ToString(), log);

		Index++;
	}
}

void AKnowledgeGraph::calculate_charge_force_and_update_velocity()
{
	bool log = true;
	bool log2 = false;


	if (!Config.bUseBruteForceForManyBody)
	{
		//
		OctreeData2 = new OctreeNode(
		);


		OctreeData2->AddAll1(
			GraphNodes,
			nodePositions);

		OctreeData2->AccumulateStrengthAndComputeCenterOfMass();

		// LogAlways("tttttttttttttttttttttttt");
		LogMessage("!!!OctreeData2->CenterOfMass: " + OctreeData2->CenterOfMass.ToString(), log);
		LogMessage("!!!OctreeData2->strength: " + FString::SanitizeFloat(OctreeData2->Strength), log);


		if (!Config.bUseParallelProcessing)
		{
			int32 Index = 0;
			for (auto& node : GraphNodes)
			{
				LogMessage("--------------------------------------", log);
				LogMessage(
					"Traverse the tree And calculate velocity on this Actor Kn, nodekey: -"
					+
					FString::FromInt(
						Index
					), log);


				TraverseBFS(OctreeData2,
				            SampleCallback,
				            Config.Alpha,
				            Index
				            ,
				            nodePositions,
				            nodeVelocities
				);
				LogMessage("Finished traversing the tree based on this Actor Kn. ", log);
				Index++;
			}
		}
		else
		{
			ParallelFor(GraphNodes.Num(), [&](int32 Index)
			{
				TraverseBFS(OctreeData2,
				            SampleCallback, Config.Alpha, Index, nodePositions, nodeVelocities);
			});
		}


		LogMessage("Finished traversing, now we can delete the tree. ", log);
		delete OctreeData2;
	}
	else
	{
		if (!Config.bUseParallelProcessing)
		{
			// Brute force
			int32 Index = 0;
			for (auto& node : GraphNodes)
			{
				auto kn = node.node;


				int32 Index2 = 0;

				for (auto& node2 : GraphNodes)
				{
					auto kn2 = node2.node;
					if (kn != kn2)
					{
						// FVector dir = kn2->GetActorLocation() - kn->GetActorLocation();
						FVector dir = nodePositions[
							Index2] - nodePositions[Index];

						float l = dir.Size() * dir.Size();
						if (l < Config.DistanceMin)
						{
							l = sqrt(Config.DistanceMin * l);
						}
						nodeVelocities[Index] += dir * Config.NodeStrength * Config.Alpha / l;
						// kn->velocity += dir * nodeStrength * alpha / l; 
					}
					Index2++;
				}


				Index++;
			}
		}
		else
		{
			ParallelFor(GraphNodes.Num(), [&](int32 Index)
			{
				auto node = GraphNodes[Index];


				int32 Index2 = 0;
				for (auto& node2 : GraphNodes)
				{
					auto kn2 = node2.node;
					if (node.node != kn2)
					{
						// FVector dir = kn2->GetActorLocation() - kn->GetActorLocation();
						FVector dir = nodePositions[Index2] - nodePositions[Index];

						float l = dir.Size() * dir.Size();
						if (l < Config.DistanceMin)
						{
							l = sqrt(Config.DistanceMin * l);
						}
						nodeVelocities[Index] += dir * Config.NodeStrength * Config.Alpha * Config.UniversalGraphScale / l;
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
	for (auto& node : GraphNodes)
	{
		// aggregation += node.Value->GetActorLocation();
		aggregation += nodePositions[
			Index
		];

		Index++;
	}

	Index = 0;
	for (auto& node : GraphNodes)
	{
		nodePositions[
				Index
			] =
			nodePositions[
				Index
			] - (
				aggregation / GraphNodes.Num() - center
			) * 1;
		Index++;
	}


	LogMessage("Ignoring the update position step for now in the center force. ");
}


void AKnowledgeGraph::update_position_array_according_to_velocity_array()
{
	if (!Config.bUseParallelProcessing)
	{
		int32 Index = 0;
		for (auto& node : GraphNodes)
		{
			auto kn = node.node;

			nodeVelocities[
				Index
			] *= Config.VelocityDecay;
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

		ParallelFor(GraphNodes.Num(), [&](int32 Index)
		{
			nodeVelocities[Index] *= Config.VelocityDecay;
			nodePositions[Index] = nodePositions[Index] + nodeVelocities[Index];
		});
	}
}

void AKnowledgeGraph::update_link_position()
{
	for (auto& link : GraphLinks)
	{
		FVector Location1 = nodePositions[link.SourceNodeIndex];
		FVector Location2 = nodePositions[link.target];


		if (Config.bUseLinkStaticMesh)
		{
			auto l = link.edgeMesh;

			// Safety check - skip if mesh is null
			if (!l)
			{
				continue;
			}

			FVector MidPoint = (Location1 + Location2) / 2.0f;
			FVector ForwardVector = (Location2 - Location1);
			float CylinderHeight = ForwardVector.Size();

			FRotator Rotation = FRotationMatrix::MakeFromZ(ForwardVector).Rotator();

			l->SetWorldLocation(
				Location1
			);

			l->SetWorldScale3D(
				FVector(
					Config.LinkThickness,
					Config.LinkThickness,
					Config.LinkLengthFineTune * CylinderHeight)
			);
			l->SetWorldRotation(
				Rotation
			);
		}
		if (Config.bUseLinkDebugLine)
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


	if (Config.bCalculateLinkForce)
	{
		LogMessage("Ready to calculate link.--------------------------------------", log);
		calculate_link_force_and_update_velocity();
		LogMessage("Finish calculating link.--------------------------------------", log);
	}
	else
	{
		LogMessage("Config.bCalculateLinkForce is disabled. ", log);
	}


	if (Config.bCalculateManyBodyForce)
	{
		LogMessage("Ready to calculate charge.--------------------------------------", log);

		calculate_charge_force_and_update_velocity();
		LogMessage("Finish calculating charge.--------------------------------------", log);
	}
	else
	{
		LogMessage("Config.bCalculateManyBodyForce is disabled. ", log);
	}


	if (true)
	{
		LogMessage("centre force is disabled for debugging. ", log);
	}
	else
	{
		calculate_centre_force_and_update_position();
	}
}


void AKnowledgeGraph::initialize_node_position()
{
	if (Config.bInitializeUsingActorLocation)
	{
		current_own_position = GetActorLocation();
	}

	if (!Config.bUseParallelProcessing)
	{
		for (
			int32 index = 0; index < TotalNodeCount; index++
		)
		{
			initialize_node_position_individual(
				index);
		}
	}
	else
	{
		ParallelFor(
			TotalNodeCount, [&](int32 index)
			{
				initialize_node_position_individual(
					index);
			}
		);
	}

	if (Config.bUseInstancedStaticMesh)
	{
		InstancedStaticMeshComponent->AddInstances(BodyTransforms, false);
	}
}

void AKnowledgeGraph::initialize_node_position_individual(int index)
{
	// Fibonacci/golden angle spiral distribution for initial node positions
	// Reference: d3-force initial positioning algorithm (https://github.com/d3/d3-force)
	// This creates an evenly distributed spiral pattern that minimizes initial overlaps
	// DO NOT MODIFY - based on d3-force implementation
	
	FVector init_pos;

	if (Config.bUsePredefinedLocation)
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
			radius = Config.InitialRadius * cbrt(0.5f + index);
		}
		else if (nDim > 1)
		{
			radius = Config.InitialRadius * sqrt(0.5f + index);
		}
		else
		{
			radius = Config.InitialRadius * index;
		}

		// Golden angle (137.5 degrees in radians) for optimal spiral distribution
		// DO NOT MODIFY - this is the mathematical golden angle from d3-force
		float initialAngleRoll = Config.InitialAngle; // Roll angle

		// Yaw angle for 3D spherical distribution
		// DO NOT MODIFY - from d3-force 3D implementation
		float initialAngleYaw = PI * 20 / (9 + sqrt(221)); // Yaw angle if needed (3D)


		float rollAngle = index * initialAngleRoll; // Roll angle
		float yawAngle = index * initialAngleYaw; // Yaw angle if needed (3D)


		if (nDim == 1)
		{
			// 1D: Positions along X axis
			init_pos = FVector(
				radius * Config.UniversalGraphScale,
				0,
				0);
		}
		else if (nDim == 2)
		{
			// 2D: Circular distribution
			init_pos = FVector(
				radius * cos(rollAngle) * Config.UniversalGraphScale,
				radius * sin(rollAngle) * Config.UniversalGraphScale,
				0
			);
		}
		else
		{
			// 3D: Spherical distribution
			init_pos = FVector(
				radius * sin(rollAngle) * cos(yawAngle) * Config.UniversalGraphScale,
				radius * cos(rollAngle) * Config.UniversalGraphScale,
				radius * sin(rollAngle) * sin(yawAngle) * Config.UniversalGraphScale
			);
		}

		if (Config.bInitializeUsingActorLocation)
		{
			init_pos += current_own_position;
		}
	}

	nodePositions[index] = init_pos;

	LogMessage("index: " + FString::FromInt(index) + " init_pos: " + init_pos.ToString());

	if (Config.bUseInstancedStaticMesh)
	{
		float s = Config.InstancedMeshSize;
		FTransform MeshTransform(
			FRotator(),
			init_pos,
			FVector(s, s, s)
		);
		BodyTransforms[index] = MeshTransform;
	}

	if (Config.bUseGPUShaders)
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

// Functions moved to KnowledgeGraph_Utilities.cpp




void AKnowledgeGraph::update_node_world_position_according_to_position_array()
{
	if (Config.bUseGPUShaders && !GPUvalid)
	{
		return;
	}

	// Update bodies visual with new positions.
	for (int i = 0; i < nodePositions.Num(); i++)
	{
		FVector NewPosition = nodePositions[i];

		
		if (Config.bUseInstancedStaticMesh)
		{
			BodyTransforms[i].SetTranslation(NewPosition);
		}

		if (Config.bUseTextRenderComponents)
		{
			GraphNodes[i].textComponent->SetWorldLocation(NewPosition);
			// TextComponents11111111111111111111[i]->SetWorldLocation(NewPosition);
			
		}
	}

	if (Config.bUseInstancedStaticMesh)
	{
		InstancedStaticMeshComponent->BatchUpdateInstancesTransforms(0, BodyTransforms, false, true);
	}
}

void AKnowledgeGraph::calculate_bias_and_strength_of_links()
{
	// Calculate link bias and strength based on node degree (number of connections)
	// Reference: d3-force link force implementation (https://github.com/d3/d3-force)
	// Bias: determines how force is distributed between source and target nodes
	// Strength: inversely proportional to node degree (highly connected nodes have weaker individual links)
	// DO NOT MODIFY - based on d3-force implementation
	
	bool log = false;
	float n = GraphNodes.Num();
	float m = GraphLinks.Num();

	std::map<int32, int32> Nodeconnection;

	std::map<int, std::vector<int>> connectout;
	std::map<int, std::vector<int>> connectin;


	if (Config.bUseGPUShaders)
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

	for (auto& link : GraphLinks)
	{
		Nodeconnection[link.SourceNodeIndex] += 1;
		Nodeconnection[link.target] += 1;

		if (Config.bUseGPUShaders)
		{
			connectout[link.SourceNodeIndex].push_back(link.target);
			connectin[link.target].push_back(link.SourceNodeIndex);
		}
	}


	if (!Config.bUseGPUShaders)
	{
		int i = 0;
		for (auto& link : GraphLinks)
		{
			int s1 = Nodeconnection[link.SourceNodeIndex];
			int s2 = Nodeconnection[link.target];

			float ttttttttttt = s1 + s2;
			
			// Bias: ratio of source degree to total degree (source + target)
			// This determines how the link force is distributed between nodes
			// DO NOT MODIFY - from d3-force
			float bias = s1 / ttttttttttt;

			link.bias = bias;

			// Strength: 1 / min(source_degree, target_degree)
			// This prevents highly connected nodes from dominating the layout
			// DO NOT MODIFY - from d3-force
			link.strength = 1.0 / fmin(s1,
			                           s2);
			LogMessage("i: " + FString::FromInt(i), log);
			LogMessage("link.bias: " + FString::SanitizeFloat(link.bias), log);
			LogMessage("link.strength: " + FString::SanitizeFloat(link.strength), log);
			i++;
		}
	}
	else
	{
		int32 Index = 0;
		for (int i = 0; i < n; i++)
		{
			LogMessage("i: " + FString::FromInt(i), log);

			int outcount = connectout[i].size();
			int incount = connectin[i].size();

			LogMessage("outcount: " + FString::FromInt(outcount), log);
			LogMessage("incount: " + FString::FromInt(incount), log);

			int totalcount = Nodeconnection[i];

			if (totalcount != outcount + incount)
			{
				LogMessage("totalcount!=outcount+incount", true, 2);
				QuitGame();
			}


			LinkOffsets[i] = Index;

			LogMessage("LinkOffsets[i]: " + FString::FromInt(LinkOffsets[i]), log);

			LinkCounts[i] = Nodeconnection[i];
			LogMessage("LinkCounts[i]: " + FString::FromInt(LinkCounts[i]), log);

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
		LogMessage("LinkOffsets: " + ConcatenatedString, log);

		ConcatenatedString = "";
		for (int32 Number : LinkCounts)
		{
			ConcatenatedString += FString::Printf(TEXT("%d "), Number);
		}
		LogMessage("LinkCounts: " + ConcatenatedString, log);

		ConcatenatedString = "";
		for (int32 Number : LinkIndices)
		{
			ConcatenatedString += FString::Printf(TEXT("%d "), Number);
		}
		LogMessage("LinkIndices: " + ConcatenatedString, log);

		ConcatenatedString = "";
		for (float Number : LinkStrengths)
		{
			ConcatenatedString += FString::Printf(TEXT("%f "), Number);
		}
		LogMessage("LinkStrengths: " + ConcatenatedString, log);

		ConcatenatedString = "";
		for (float Number : LinkBiases)
		{
			ConcatenatedString += FString::Printf(TEXT("%f "), Number);
		}
		LogMessage("LinkBiases: " + ConcatenatedString, log);

		ConcatenatedString = "";
		for (int32 Number : Linkinout)
		{
			ConcatenatedString += FString::Printf(TEXT("%d "), Number);
		}
		LogMessage("Linkinout: " + ConcatenatedString, log);


		SimParameters.LinkOffsets = LinkOffsets;
		SimParameters.LinkCounts = LinkCounts;
		SimParameters.LinkIndices = LinkIndices;
		SimParameters.LinkStrengths = LinkStrengths;
		SimParameters.LinkBiases = LinkBiases;
		SimParameters.Linkinout = Linkinout;
	}
}


bool AKnowledgeGraph::generate_actor_for_a_link(GraphLink& link)
{
	AKnowledgeEdge* e;
	UClass* bpClass;

	bool useCorrectWay=true;
	if (useCorrectWay)
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
			LogMessage("generate_actor_for_a_link failed to load class", true, 2);
			QuitGame();
			return true;
			LogMessage("error loading classsssssssssssssssssssssss");
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
			GraphSystemDebugBreak();
		}
		// Check if the Blueprint class is valid
		bpClass = LoadedBP->GeneratedClass;
		if (!bpClass)
		{
			UE_LOG(LogTemp, Error, TEXT("Generated class from Blueprint is null."));
			GraphSystemDebugBreak();
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
	GraphLink link = GraphLink(source, target);

	if (Config.bUseLinkStaticMesh)
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
			Config.LinkMesh
		);

		CylinderMesh->SetMaterial(0, Config.LinkMaterial);

		
		link.edgeMesh = CylinderMesh;
	}

	// Default link properties from d3-force
	// DO NOT MODIFY - these are reference values from d3-force
	link.strength = 1; // Will be recalculated based on node degree
	link.distance = Config.EdgeDistance; // Default 30 from d3-force

	GraphLinks[id] = link;
}
