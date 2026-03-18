#include "KnowledgeGraph.h"

#include <random>


#include "NBodyUtils.h"
#include "GameFramework/Character.h"


void AKnowledgeGraph::GenerateTextRenderComponentAndAttach(FString name,int32 index)
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
		// GraphNodeTextComponents.Add(TextComponent);
	}
}

void AKnowledgeGraph::GetNumberOfNodes()
{
	if (Config.CreationMode == EGraphCreationMode::AutoGenerate)
	{
		LogMessage("Generating graph automatically. Number of nodes: " + FString::FromInt(Config.AutoGenerateNodeCount), true, 0,
		   TEXT("GetNumberOfNodes: "));
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

void AKnowledgeGraph::CreateOneToOneMapping()
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

void AKnowledgeGraph::Miscellaneous()
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
			AddEdge(jid, jsource, jtarget);
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
			AddEdge(jid, jsource, jtarget);
		}
	}
}

void AKnowledgeGraph::SetArrayLengths()
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

void AKnowledgeGraph::SetArrayValues()
{
	for (FVector& velocity : nodeVelocities)
	{
		velocity.X = 0.0f;
		velocity.Y = 0.0f;
		velocity.Z = 0.0f;
	}
}

void AKnowledgeGraph::InitializeArrays()
{
	SetArrayLengths();
	SetArrayValues();
}

bool AKnowledgeGraph::GenerateObjectsForNodeAndLink()
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
				GenerateTextRenderComponentAndAttach(name,i);
			}
		}
		Miscellaneous();
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
					LogToScreen("WARNING: Failed to get node name from JSON at index " + FString::FromInt(i) + ", using fallback", true, 2);
					name = "Sample Text : " + FString::FromInt(i);
				}
				GenerateTextRenderComponentAndAttach(name,i);
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
			AddEdge(i, jsource, jtarget);
		}

		LogMessage("Number of link generated: " + FString::FromInt(jedges.Num()), log);
	}
	return false;
}

void AKnowledgeGraph::DealWithPredefinedLocation()
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

void AKnowledgeGraph::DefaultGenerateGraphMethod()
{

	GetNumberOfNodes();

	if (
		Config.CreationMode == EGraphCreationMode::FromJson || Config.CreationMode == EGraphCreationMode::FromDatabase
	)
	{
		LogMessage("creating one to one mapping", true, 0, TEXT("DefaultGenerateGraphMethod: "));
		CreateOneToOneMapping();
	}else
	{
		LogMessage("auto generate graph, no need to create one to one mapping", true, 0, TEXT("DefaultGenerateGraphMethod: "));
	}
	
	InitializeArrays();
	
	if (Config.CreationMode == EGraphCreationMode::FromDatabase)
	{
		// Node properties are now handled by DataManager
	}else
	{
		
	}
	
	if (GenerateObjectsForNodeAndLink())
	{
		return;
	}

	if (Config.bUsePredefinedLocation)
	{	
		DealWithPredefinedLocation();
	}else
	{
		LogMessage("not using predefined location", true, 0, TEXT("DefaultGenerateGraphMethod: "));
	}

	LogMessage("post generate graph", true, 0, TEXT("DefaultGenerateGraphMethod: "));
	PostGenerateGraph();
}


void AKnowledgeGraph::CalculateLinkForceAndUpdateVelocity()
{
	bool log = Config.bEnableLogging;


	// link forces
	// After loop, the velocity of all notes have been altered a little bit because of the link force already. 
	int32 Index = 0;
	for (auto& link : GraphLinks)
	{
		LogMessage("link iteration: !!!!!!!!!!!!!!!!!!" + FString::FromInt(Index), log);

		FVector source_pos = nodePositions[link.SourceNodeIndex];
		FVector source_velocity = nodeVelocities[link.SourceNodeIndex];
		FVector target_pos = nodePositions[link.TargetNodeIndex];
		FVector target_velocity = nodeVelocities[link.TargetNodeIndex];

		FVector new_v = target_pos + target_velocity - source_pos - source_velocity;


		LogMessage("new_v: " + new_v.ToString(), log);
		LogMessage("target_pos- source_pos: " + (target_pos - source_pos).ToString(), log);
		if (Config.bEnableJiggle)
		{
			if (new_v.IsNearlyZero())
			{
				new_v = Jiggle(new_v, 1e-6f);
			}
			LogMessage("GIGGLE is enabled............", log);
		}

		float l = new_v.Size();

		// TODO: Add division-by-zero guard for l == 0
		// By looking at the javascript code, we can see strength Will only be computed when there is a change Of the graph structure to the graph.
		l = (l - link.LinkDistance * Config.UniversalGraphScale) /
			l
			* Config.Alpha
			* link.LinkStrength;
		new_v *= l;

		LogMessage("before update nodeVelocities", log);
		LogMessage("nodeVelocities[" + FString::FromInt(link.TargetNodeIndex) + "]: " + nodeVelocities[link.TargetNodeIndex].ToString(), log);
		LogMessage("nodeVelocities[" + FString::FromInt(link.SourceNodeIndex) + "]: " + nodeVelocities[link.SourceNodeIndex].ToString(), log);
		nodeVelocities[link.TargetNodeIndex] -= new_v * (link.LinkBias);
		nodeVelocities[link.SourceNodeIndex] += new_v * (1 - link.LinkBias);

		LogMessage("after update nodeVelocities", log);
		LogMessage("nodeVelocities[" + FString::FromInt(link.TargetNodeIndex) + "]: " + nodeVelocities[link.TargetNodeIndex].ToString(), log);
		LogMessage("nodeVelocities[" + FString::FromInt(link.SourceNodeIndex) + "]: " + nodeVelocities[link.SourceNodeIndex].ToString(), log);

		Index++;
	}
}

void AKnowledgeGraph::CalculateChargeForceAndUpdateVelocity()
{
	bool log = Config.bEnableLogging;
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
			for (int32 NodeIdx = 0; NodeIdx < GraphNodes.Num(); NodeIdx++)
			{
				for (int32 OtherNodeIdx = 0; OtherNodeIdx < GraphNodes.Num(); OtherNodeIdx++)
				{
					if (NodeIdx != OtherNodeIdx)
					{
						FVector dir = nodePositions[OtherNodeIdx] - nodePositions[NodeIdx];

						float l = dir.Size() * dir.Size();
						if (l < Config.DistanceMin)
						{
							l = sqrt(Config.DistanceMin * l);
						}
						nodeVelocities[NodeIdx] += dir * Config.NodeStrength * Config.Alpha / l;
					}
				}
			}
		}
		else
		{
			ParallelFor(GraphNodes.Num(), [&](int32 NodeIdx)
			{
				for (int32 OtherNodeIdx = 0; OtherNodeIdx < GraphNodes.Num(); OtherNodeIdx++)
				{
					if (NodeIdx != OtherNodeIdx)
					{
						FVector dir = nodePositions[OtherNodeIdx] - nodePositions[NodeIdx];

						float l = dir.Size() * dir.Size();
						if (l < Config.DistanceMin)
						{
							l = sqrt(Config.DistanceMin * l);
						}
						nodeVelocities[NodeIdx] += dir * Config.NodeStrength * Config.Alpha * Config.UniversalGraphScale / l;
					}
				}
			});
		}
	}
}

void AKnowledgeGraph::CalculateCentreForceAndUpdatePosition()
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
	// TODO: Add division-by-zero guard for GraphNodes.Num() == 0
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


void AKnowledgeGraph::UpdatePositionArrayAccordingToVelocityArray()
{
	if (!Config.bUseParallelProcessing)
	{
		for (int32 NodeIdx = 0; NodeIdx < GraphNodes.Num(); NodeIdx++)
		{
			nodeVelocities[NodeIdx] *= Config.VelocityDecay;
			nodePositions[NodeIdx] = nodePositions[NodeIdx] + nodeVelocities[NodeIdx];
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

void AKnowledgeGraph::UpdateLinkPosition()
{
	for (auto& link : GraphLinks)
	{
		FVector Location1 = nodePositions[link.SourceNodeIndex];
		FVector Location2 = nodePositions[link.TargetNodeIndex];


		if (Config.bUseLinkStaticMesh)
		{
			auto l = link.EdgeMeshComponent;

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

void AKnowledgeGraph::ApplyForce()
{
	bool log = Config.bEnableLogging;

	// In here velocity of all notes are zeroed
	// In the following for loop, In the first few loop, the velocity is 0. 


	if (Config.bCalculateLinkForce)
	{
		LogMessage("Ready to calculate link.--------------------------------------", log);
		CalculateLinkForceAndUpdateVelocity();
		LogMessage("Finish calculating link.--------------------------------------", log);
	}
	else
	{
		LogMessage("Config.bCalculateLinkForce is disabled. ", log);
	}


	if (Config.bCalculateManyBodyForce)
	{
		LogMessage("Ready to calculate charge.--------------------------------------", log);

		CalculateChargeForceAndUpdateVelocity();
		LogMessage("Finish calculating charge.--------------------------------------", log);
	}
	else
	{
		LogMessage("Config.bCalculateManyBodyForce is disabled. ", log);
	}


	if (!Config.bCalculateCenterForce)
	{
		LogMessage("centre force is disabled. ", log);
	}
	else
	{
		CalculateCentreForceAndUpdatePosition();
	}
}


void AKnowledgeGraph::InitializeNodePosition()
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
			InitializeNodePositionIndividual(
				index);
		}
	}
	else
	{
		ParallelFor(
			TotalNodeCount, [&](int32 index)
			{
				InitializeNodePositionIndividual(
					index);
			}
		);
	}

	if (Config.bUseInstancedStaticMesh)
	{
		InstancedStaticMeshComponent->AddInstances(BodyTransforms, false);
	}
}

void AKnowledgeGraph::InitializeNodePositionIndividual(int index)
{
	// Fibonacci/golden angle spiral distribution for initial node positions
	// Reference: d3-force initial positioning algorithm (https://github.com/d3/d3-force)
	// This creates an evenly distributed spiral pattern that minimizes initial overlaps
	// DO NOT MODIFY - based on d3-force implementation
	
	FVector init_pos;

	if (Config.bUsePredefinedLocation && predefined_positions.Num() > index)
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




void AKnowledgeGraph::UpdateNodeWorldPositionAccordingToPositionArray()
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
			// GraphNodeTextComponents[i]->SetWorldLocation(NewPosition);
			
		}
	}

	if (Config.bUseInstancedStaticMesh)
	{
		InstancedStaticMeshComponent->BatchUpdateInstancesTransforms(0, BodyTransforms, false, true);
	}
}

void AKnowledgeGraph::CalculateBiasAndStrengthOfLinks()
{
	// Calculate link bias and strength based on node degree (number of connections)
	// Reference: d3-force link force implementation (https://github.com/d3/d3-force)
	// Bias: determines how force is distributed between source and target nodes
	// Strength: inversely proportional to node degree (highly connected nodes have weaker individual links)
	// DO NOT MODIFY - based on d3-force implementation
	
	bool log = false;
	float n = GraphNodes.Num();
	float m = GraphLinks.Num();

	TMap<int32, int32> Nodeconnection;

	TMap<int32, TArray<int32>> connectout;
	TMap<int32, TArray<int32>> connectin;


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
		Nodeconnection.FindOrAdd(link.SourceNodeIndex, 0) += 1;
		Nodeconnection.FindOrAdd(link.TargetNodeIndex, 0) += 1;

		if (Config.bUseGPUShaders)
		{
			connectout[link.SourceNodeIndex].Add(link.TargetNodeIndex);
			connectin[link.TargetNodeIndex].Add(link.SourceNodeIndex);
		}
	}


	if (!Config.bUseGPUShaders)
	{
		int i = 0;
		for (auto& link : GraphLinks)
		{
			int s1 = Nodeconnection[link.SourceNodeIndex];
			int s2 = Nodeconnection[link.TargetNodeIndex];

			float TotalDegree = s1 + s2;
			
			// TODO: Add division-by-zero guard for TotalDegree == 0 and fmin(s1, s2) == 0
			// Bias: ratio of source degree to total degree (source + target)
			// This determines how the link force is distributed between nodes
			// DO NOT MODIFY - from d3-force
			float bias = s1 / TotalDegree;

			link.LinkBias = bias;

			// Strength: 1 / min(source_degree, target_degree)
			// This prevents highly connected nodes from dominating the layout
			// DO NOT MODIFY - from d3-force
			link.LinkStrength = 1.0 / fmin(s1,
			                           s2);
			LogMessage("i: " + FString::FromInt(i), log);
			LogMessage("link.LinkBias: " + FString::SanitizeFloat(link.LinkBias), log);
			LogMessage("link.LinkStrength: " + FString::SanitizeFloat(link.LinkStrength), log);
			i++;
		}
	}
	else
	{
		int32 Index = 0;
		for (int i = 0; i < n; i++)
		{
			LogMessage("i: " + FString::FromInt(i), log);

			int outcount = connectout.Contains(i) ? connectout[i].Num() : 0;
			int incount = connectin.Contains(i) ? connectin[i].Num() : 0;

			LogMessage("outcount: " + FString::FromInt(outcount), log);
			LogMessage("incount: " + FString::FromInt(incount), log);

			int totalcount = Nodeconnection.Contains(i) ? Nodeconnection[i] : 0;

			if (totalcount != outcount + incount)
			{
				LogMessage("totalcount!=outcount+incount", true, 2);
				QuitGame();
			}


			LinkOffsets[i] = Index;

			LogMessage("LinkOffsets[i]: " + FString::FromInt(LinkOffsets[i]), log);

			LinkCounts[i] = Nodeconnection.Contains(i) ? Nodeconnection[i] : 0;
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

				int s1 = Nodeconnection.Contains(i) ? Nodeconnection[i] : 0;
				int s2 = Nodeconnection.Contains(connectout[i][j]) ? Nodeconnection[connectout[i][j]] : 0;

				float TotalDegree = s1 + s2;
				float bias = s1 / TotalDegree;
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

				int s2 = Nodeconnection.Contains(i) ? Nodeconnection[i] : 0;
				int s1 = Nodeconnection.Contains(counterpart) ? Nodeconnection[counterpart] : 0;

				float TotalDegree = s1 + s2;
				float bias = s1 / TotalDegree;
				LinkBiases[indexnow] = bias;
				LinkStrengths[indexnow] = 1.0 / fmin(s1,
				                                     s2);
			}
			Index += Nodeconnection.Contains(i) ? Nodeconnection[i] : 0;
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


void AKnowledgeGraph::AddEdge(int32 id, int32 source, int32 target)
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

		
		link.EdgeMeshComponent = CylinderMesh;
	}

	// Default link properties from d3-force
	// DO NOT MODIFY - these are reference values from d3-force
	link.LinkStrength = 1; // Will be recalculated based on node degree
	link.LinkDistance = Config.EdgeDistance; // Default 30 from d3-force

	GraphLinks[id] = link;
}
