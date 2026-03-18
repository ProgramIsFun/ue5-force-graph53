// KnowledgeGraph_BlueprintAPI.cpp
//
// Blueprint-callable functions for user interaction and graph editing.
// Provides mode-aware routing:
//   - AutoGenerate / FromJson: edits apply locally and immediately
//   - FromDatabase: HTTP request first, local edit only on server success
//
// Key Functions:
// - RequestAddGraphNode(): Mode-aware node addition
// - RequestRemoveSelectedGraphNode(): Mode-aware node removal
// - AddGraphNodeLocal() / RemoveGraphNodeByIndex(): Low-level swap-remove operations
// - SelectClosestGraphNodeToPlayer(): Find nearest node to player
// - SyncGraphNodePositionsToDatabase(): Bulk position sync
//
// Part of the KnowledgeGraph refactoring - extracted from KnowledgeGraph5.cpp

#include "KnowledgeGraph.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

void AKnowledgeGraph::SelectClosestGraphNodeToPlayer()
{
	FVector player_location = GetPlayerLocation();
	float min_distance = FLT_MAX;
	int32 closest_node_index = -1;

	if (nodePositions.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("nodePositions array is empty!"));
		return;
	}

	for (int i = 0; i < nodePositions.Num(); i++)
	{
		float distance = FVector::Dist(player_location, nodePositions[i]);

		if (distance < min_distance)
		{
			min_distance = distance;
			closest_node_index = i;
		}
	}

	if (closest_node_index != -1)
	{
		SelectedGraphNodeIndex = closest_node_index;
		SelectedGraphNodeName = NodeIdToStringMap[closest_node_index];
		LogToScreen("The closest node index is: " + FString::FromInt(closest_node_index) + 
		   " with a distance of: " + FString::SanitizeFloat(min_distance), true, 2);
		OnGraphNodeSelectionChanged.Broadcast(SelectedGraphNodeIndex);
	}
	else
	{
		LogToScreen("Could not find the closest node.", true, 2);
	}
}

void AKnowledgeGraph::SelectGraphNodeByLookAt()
{
	if (nodePositions.Num() == 0)
	{
		LogToScreen("SelectGraphNodeByLookAt: no nodes in graph.");
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController) return;

	// Get camera location and forward direction (the look-at ray)
	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
	FVector CameraForward = CameraRotation.Vector();

	float SmallestRayDistance = FLT_MAX;
	int32 ClosestNodeToRayIndex = -1;

	for (int32 i = 0; i < nodePositions.Num(); i++)
	{
		// Point-to-line distance: project node onto the camera ray
		FVector CameraToNode = nodePositions[i] - CameraLocation;
		float ProjectionLength = FVector::DotProduct(CameraToNode, CameraForward);

		// Skip nodes behind the camera
		if (ProjectionLength < 0.0f) continue;

		FVector ClosestPointOnRay = CameraLocation + CameraForward * ProjectionLength;
		float PerpendicularDistance = FVector::Dist(nodePositions[i], ClosestPointOnRay);

		if (PerpendicularDistance < SmallestRayDistance)
		{
			SmallestRayDistance = PerpendicularDistance;
			ClosestNodeToRayIndex = i;
		}
	}

	if (ClosestNodeToRayIndex < 0)
	{
		LogToScreen("SelectGraphNodeByLookAt: no node found in front of camera.");
		return;
	}

	// --- Deselect previous node (reset to white) ---
	if (SelectedGraphNodeIndex >= 0 && GraphNodes.IsValidIndex(SelectedGraphNodeIndex))
	{
		if (Config.bUseTextRenderComponents && GraphNodes[SelectedGraphNodeIndex].textComponent)
		{
			GraphNodes[SelectedGraphNodeIndex].textComponent->SetTextRenderColor(FColor::White);
		}
	}

	// --- Select new node (highlight yellow) ---
	SelectedGraphNodeIndex = ClosestNodeToRayIndex;
	SelectedGraphNodeName = NodeIdToStringMap.Contains(ClosestNodeToRayIndex)
		? NodeIdToStringMap[ClosestNodeToRayIndex]
		: FString::Printf(TEXT("Index_%d"), ClosestNodeToRayIndex);

	if (Config.bUseTextRenderComponents && GraphNodes[ClosestNodeToRayIndex].textComponent)
	{
		GraphNodes[ClosestNodeToRayIndex].textComponent->SetTextRenderColor(FColor::Yellow);
	}

	LogToScreen("Selected: " + SelectedGraphNodeName + " (idx " + FString::FromInt(ClosestNodeToRayIndex) +
		", ray dist: " + FString::SanitizeFloat(SmallestRayDistance) + ")");

	OnGraphNodeSelectionChanged.Broadcast(SelectedGraphNodeIndex);
}

// ---------------------------------------------------------------------------
// Laser-select: hold PgUp to aim a visible laser, release to confirm
// ---------------------------------------------------------------------------

void AKnowledgeGraph::BeginLaserSelect()
{
	bLaserSelectActive = true;
}

void AKnowledgeGraph::EndLaserSelect()
{
	bLaserSelectActive = false;
	// Fire the actual selection on release
	SelectGraphNodeByLookAt();
}

void AKnowledgeGraph::DrawLaserSelectRay()
{
	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController) return;

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
	FVector CameraForward = CameraRotation.Vector();

	// Find the closest node to the ray so we can draw the laser to it
	float SmallestRayDistance = FLT_MAX;
	int32 ClosestNodeToRayIndex = -1;
	FVector ClosestNodePosition = FVector::ZeroVector;

	for (int32 i = 0; i < nodePositions.Num(); i++)
	{
		FVector CameraToNode = nodePositions[i] - CameraLocation;
		float ProjectionLength = FVector::DotProduct(CameraToNode, CameraForward);
		if (ProjectionLength < 0.0f) continue;

		FVector ClosestPointOnRay = CameraLocation + CameraForward * ProjectionLength;
		float PerpendicularDistance = FVector::Dist(nodePositions[i], ClosestPointOnRay);

		if (PerpendicularDistance < SmallestRayDistance)
		{
			SmallestRayDistance = PerpendicularDistance;
			ClosestNodeToRayIndex = i;
			ClosestNodePosition = nodePositions[i];
		}
	}

	// Draw the laser line (single frame lifetime so it redraws each tick)
	constexpr float LaserRayLength = 50000.0f;
	FVector LaserEndPoint = CameraLocation + CameraForward * LaserRayLength;
	FColor LaserColor = FColor::Red;

	if (ClosestNodeToRayIndex >= 0)
	{
		// Draw laser from camera to the targeted node
		LaserEndPoint = ClosestNodePosition;
		LaserColor = FColor::Cyan;
	}

	DrawDebugLine(World, CameraLocation, LaserEndPoint, LaserColor, false, -1.0f, 0, 3.0f);

	// Draw a small sphere at the hit point for visibility
	if (ClosestNodeToRayIndex >= 0)
	{
		DrawDebugSphere(World, ClosestNodePosition, 30.0f, 8, FColor::Yellow, false, -1.0f, 0, 2.0f);
	}
}

// ---------------------------------------------------------------------------
// High-level mode-aware graph editing
// ---------------------------------------------------------------------------
// AutoGenerate / FromJson  → edit locally and immediately
// FromDatabase             → HTTP request first, local edit only on success
// ---------------------------------------------------------------------------

void AKnowledgeGraph::RequestAddGraphNode(FString NodeName, int32 LinkTargetNodeIndex)
{
	if (Config.CreationMode == EGraphCreationMode::FromDatabase)
	{
		// Database mode: send to server first, apply locally in callback
		AddGraphNodeToDatabase(NodeName);
	}
	else
	{
		// AutoGenerate / FromJson: apply immediately
		// Spawn 500 units (5m) in front of the player so the node is visible near the graph
		FVector SpawnPosition = GetPlayerLocation();
		UWorld* World = GetWorld();
		if (World)
		{
			APlayerController* PlayerController = World->GetFirstPlayerController();
			if (PlayerController && PlayerController->GetPawn())
			{
				FVector ForwardVector = PlayerController->GetPawn()->GetActorForwardVector();
				SpawnPosition += ForwardVector * 500.0f;
			}
		}
		FString GeneratedId = FString::Printf(TEXT("local_%d_%lld"), GraphNodes.Num(), FDateTime::Now().GetTicks());
		AddGraphNodeLocal(NodeName, GeneratedId, SpawnPosition, LinkTargetNodeIndex);
	}
}

void AKnowledgeGraph::RequestRemoveSelectedGraphNode()
{
	if (SelectedGraphNodeIndex < 0 || !GraphNodes.IsValidIndex(SelectedGraphNodeIndex))
	{
		LogToScreen("No valid node selected for removal.");
		return;
	}

	if (Config.CreationMode == EGraphCreationMode::FromDatabase)
	{
		// Database mode: send delete to server first, apply locally in callback
		DeleteGraphNodeFromDatabase();
	}
	else
	{
		// AutoGenerate / FromJson: apply immediately
		RemoveGraphNodeByIndex(SelectedGraphNodeIndex);
	}
}

// ---------------------------------------------------------------------------
// Database HTTP operations (FromDatabase mode only)
// ---------------------------------------------------------------------------

void AKnowledgeGraph::AddGraphNodeToDatabase(FString NodeName)
{
	FVector player_location = GetPlayerLocation();
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(Config.GraphNodeCreateUrl);
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", "application/json");

	// Create JSON Object
	TSharedPtr<FJsonObject> NodeJsonPayload = MakeShareable(new FJsonObject);
	NodeJsonPayload->SetStringField("name", NodeName);
	NodeJsonPayload->SetNumberField("locationX", player_location.X);
	NodeJsonPayload->SetNumberField("locationY", player_location.Y);
	NodeJsonPayload->SetNumberField("locationZ", player_location.Z);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(NodeJsonPayload.ToSharedRef(), Writer);

	HttpRequest->SetContentAsString(RequestBody);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &AKnowledgeGraph::OnAddGraphNodeHttpCompleted);
	HttpRequest->ProcessRequest();
}

void AKnowledgeGraph::OnAddGraphNodeHttpCompleted(
	FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response->GetResponseCode() == 200)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			FString NodeDatabaseId = JsonObject->GetStringField("id");
			FString NodeDatabaseName = JsonObject->GetStringField("name");
			FVector SpawnPosition = GetPlayerLocation();

			// Server accepted — now apply locally
			AddGraphNodeLocal(NodeDatabaseName, NodeDatabaseId, SpawnPosition, /*LinkTargetNodeIndex=*/ -1);
		}
		else
		{
			LogToScreen("Failed to parse add-node JSON response");
		}
	}
	else
	{
		FString ErrorInfo = Response.IsValid() ? Response->GetContentAsString() : TEXT("No response from server");
		LogToScreen("Failed to add node to database: " + ErrorInfo);
	}
}

void AKnowledgeGraph::DeleteGraphNodeFromDatabase()
{
	if (SelectedGraphNodeIndex < 0 || !GraphNodes.IsValidIndex(SelectedGraphNodeIndex))
	{
		LogToScreen("DeleteGraphNodeFromDatabase: no valid node selected.");
		return;
	}

	// Look up the string ID for the selected node
	FString NodeDatabaseId;
	if (NodeIdToStringMap.Contains(SelectedGraphNodeIndex))
	{
		NodeDatabaseId = NodeIdToStringMap[SelectedGraphNodeIndex];
	}
	else
	{
		LogToScreen("DeleteGraphNodeFromDatabase: selected node has no database ID.");
		return;
	}

	// TODO: Replace with your actual delete endpoint URL from Config
	FString DeleteUrl = Config.GraphDatabaseQueryUrl + "/" + NodeDatabaseId;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(DeleteUrl);
	HttpRequest->SetVerb("DELETE");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &AKnowledgeGraph::OnDeleteGraphNodeHttpCompleted);
	HttpRequest->ProcessRequest();

	LogToScreen("Requesting server to delete node: " + NodeDatabaseId);
}

void AKnowledgeGraph::OnDeleteGraphNodeHttpCompleted(
	FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
	{
		// Server accepted the delete — now apply locally
		if (GraphNodes.IsValidIndex(SelectedGraphNodeIndex))
		{
			RemoveGraphNodeByIndex(SelectedGraphNodeIndex);
		}
	}
	else
	{
		FString ErrorInfo = Response.IsValid() ? Response->GetContentAsString() : TEXT("No response from server");
		LogToScreen("Failed to delete node from database: " + ErrorInfo);
	}
}

void AKnowledgeGraph::DeleteGraphLinkFromDatabase()
{
	// TODO: Implement HTTP DELETE for link removal, then call local link removal on success
}

void AKnowledgeGraph::AddGraphLinkToDatabase()
{
	// TODO: Implement HTTP POST for link creation, then call local link addition on success
}

// ---------------------------------------------------------------------------
// Incremental graph mutation — swap-remove approach
// ---------------------------------------------------------------------------
//
// All parallel arrays (GraphNodes, nodePositions, nodeVelocities, BodyTransforms)
// are kept dense. Deleting a node at index K works by:
//   1. Destroy visuals for node K and any links referencing K
//   2. Move the last element into slot K
//   3. Fix up every GraphLink that pointed to the old last index
//   4. Shrink arrays by one
//   5. Rebuild instanced mesh instances
//   6. Recalculate link bias/strength
// ---------------------------------------------------------------------------

bool AKnowledgeGraph::RemoveGraphNodeByIndex(int32 NodeIndexToRemove)
{
	if (!GraphNodes.IsValidIndex(NodeIndexToRemove))
	{
		LogToScreen("RemoveGraphNodeByIndex: invalid index " + FString::FromInt(NodeIndexToRemove));
		return false;
	}

	const int32 LastNodeIndex = GraphNodes.Num() - 1;

	// --- 1. Destroy the visual for the node being removed ---
	if (Config.bUseTextRenderComponents && GraphNodes[NodeIndexToRemove].textComponent)
	{
		UTextRenderComponent* TextComp = GraphNodes[NodeIndexToRemove].textComponent;
		if (TextComp->IsRegistered())
		{
			TextComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			TextComp->UnregisterComponent();
			TextComp->DestroyComponent();
		}
		GraphNodes[NodeIndexToRemove].textComponent = nullptr;
	}

	// --- 2. Remove links that reference the deleted node, destroy their meshes ---
	// Walk backwards so removal doesn't skip elements
	for (int32 LinkIdx = GraphLinks.Num() - 1; LinkIdx >= 0; --LinkIdx)
	{
		GraphLink& Link = GraphLinks[LinkIdx];
		if (Link.SourceNodeIndex == NodeIndexToRemove || Link.TargetNodeIndex == NodeIndexToRemove)
		{
			// Destroy link mesh
			if (Link.EdgeMeshComponent && Link.EdgeMeshComponent->IsRegistered())
			{
				Link.EdgeMeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
				Link.EdgeMeshComponent->UnregisterComponent();
				Link.EdgeMeshComponent->DestroyComponent();
				Link.EdgeMeshComponent = nullptr;
			}
			// Swap-remove the link itself (don't shrink yet, we do it after the loop)
			GraphLinks.RemoveAtSwap(LinkIdx, 1, false);
		}
	}
	GraphLinks.Shrink();

	// --- 3. Update ID maps: remove the deleted node's mapping ---
	FString RemovedNodeStringId;
	if (NodeIdToStringMap.Contains(NodeIndexToRemove))
	{
		RemovedNodeStringId = NodeIdToStringMap[NodeIndexToRemove];
		NodeIdToStringMap.Remove(NodeIndexToRemove);
		StringToNodeIdMap.Remove(RemovedNodeStringId);
	}

	// --- 4. Swap last node into the deleted slot (unless it IS the last) ---
	if (NodeIndexToRemove != LastNodeIndex)
	{
		// Move data from last slot into the removed slot
		GraphNodes[NodeIndexToRemove] = GraphNodes[LastNodeIndex];
		nodePositions[NodeIndexToRemove] = nodePositions[LastNodeIndex];
		nodeVelocities[NodeIndexToRemove] = nodeVelocities[LastNodeIndex];

		if (Config.bUseInstancedStaticMesh && BodyTransforms.IsValidIndex(LastNodeIndex))
		{
			BodyTransforms[NodeIndexToRemove] = BodyTransforms[LastNodeIndex];
		}

		// Update ID maps for the moved node
		FString MovedNodeStringId;
		if (NodeIdToStringMap.Contains(LastNodeIndex))
		{
			MovedNodeStringId = NodeIdToStringMap[LastNodeIndex];
			NodeIdToStringMap.Remove(LastNodeIndex);
			NodeIdToStringMap.Add(NodeIndexToRemove, MovedNodeStringId);
			StringToNodeIdMap.Add(MovedNodeStringId, NodeIndexToRemove);
		}

		// Fix up all links that referenced the old last index
		for (GraphLink& Link : GraphLinks)
		{
			if (Link.SourceNodeIndex == LastNodeIndex)
			{
				Link.SourceNodeIndex = NodeIndexToRemove;
			}
			if (Link.TargetNodeIndex == LastNodeIndex)
			{
				Link.TargetNodeIndex = NodeIndexToRemove;
			}
		}
	}

	// --- 5. Shrink all parallel arrays by one ---
	const int32 NewNodeCount = LastNodeIndex; // == old Num() - 1
	GraphNodes.SetNum(NewNodeCount);
	nodePositions.SetNum(NewNodeCount);
	nodeVelocities.SetNum(NewNodeCount);
	TotalNodeCount = NewNodeCount;

	if (Config.bUseInstancedStaticMesh)
	{
		BodyTransforms.SetNum(NewNodeCount);
		// Rebuild instanced mesh from scratch (safest way to keep instance indices in sync)
		InstancedStaticMeshComponent->ClearInstances();
		if (BodyTransforms.Num() > 0)
		{
			InstancedStaticMeshComponent->AddInstances(BodyTransforms, false);
		}
	}

	// --- 6. Recalculate link bias/strength since node degrees changed ---
	if (GraphLinks.Num() > 0)
	{
		CalculateBiasAndStrengthOfLinks();
	}

	// Clear selection if it pointed to the removed node
	if (SelectedGraphNodeIndex == NodeIndexToRemove)
	{
		SelectedGraphNodeIndex = -1;
		SelectedGraphNodeName = TEXT("");
		OnGraphNodeSelectionChanged.Broadcast(SelectedGraphNodeIndex);
	}
	else if (SelectedGraphNodeIndex == LastNodeIndex)
	{
		// Selection was the node that got moved
		SelectedGraphNodeIndex = NodeIndexToRemove;
		OnGraphNodeSelectionChanged.Broadcast(SelectedGraphNodeIndex);
	}

	// Reheat simulation so the graph can re-settle
	Config.Alpha = FMath::Max(Config.Alpha, 0.3f);

	LogToScreen("Removed node. Graph now has " + FString::FromInt(TotalNodeCount) + " nodes, " + FString::FromInt(GraphLinks.Num()) + " links.");
	return true;
}

int32 AKnowledgeGraph::AddGraphNodeLocal(const FString& NodeName, const FString& NodeStringId, FVector NodeWorldPosition, int32 LinkTargetNodeIndex)
{
	const int32 NewNodeIndex = GraphNodes.Num();

	// --- 1. Expand all parallel arrays ---
	GraphNode NewNode;
	GraphNodes.Add(NewNode);
	nodePositions.Add(NodeWorldPosition);
	nodeVelocities.Add(FVector::ZeroVector);
	TotalNodeCount = GraphNodes.Num();

	// --- 2. Create text visual and set its position immediately ---
	if (Config.bUseTextRenderComponents)
	{
		FString DisplayName = NodeName.IsEmpty() ? ("Node " + FString::FromInt(NewNodeIndex)) : NodeName;
		GenerateTextRenderComponentAndAttach(DisplayName, NewNodeIndex);
		// Set world position right away so it's visible before the next physics tick
		if (GraphNodes[NewNodeIndex].textComponent)
		{
			GraphNodes[NewNodeIndex].textComponent->SetWorldLocation(NodeWorldPosition);
		}
	}

	// --- 3. Instanced mesh ---
	if (Config.bUseInstancedStaticMesh)
	{
		float MeshScale = Config.InstancedMeshSize;
		FTransform NodeMeshTransform(FRotator::ZeroRotator, NodeWorldPosition, FVector(MeshScale, MeshScale, MeshScale));
		BodyTransforms.Add(NodeMeshTransform);
		InstancedStaticMeshComponent->AddInstance(NodeMeshTransform, false);
	}

	// --- 4. ID maps ---
	if (!NodeStringId.IsEmpty())
	{
		NodeIdToStringMap.Add(NewNodeIndex, NodeStringId);
		StringToNodeIdMap.Add(NodeStringId, NewNodeIndex);
	}

	// --- 5. Optionally create a link to an existing node ---
	if (LinkTargetNodeIndex >= 0 && GraphNodes.IsValidIndex(LinkTargetNodeIndex))
	{
		const int32 NewLinkId = GraphLinks.Num();
		// AddEdge does GraphLinks[id] = link, so grow the array first
		GraphLinks.SetNum(NewLinkId + 1);
		AddEdge(NewLinkId, NewNodeIndex, LinkTargetNodeIndex);
	}

	// --- 6. Recalculate link bias/strength ---
	if (GraphLinks.Num() > 0)
	{
		CalculateBiasAndStrengthOfLinks();
	}

	// Reheat simulation
	Config.Alpha = FMath::Max(Config.Alpha, 0.3f);

	LogToScreen("Added node '" + NodeName + "' at index " + FString::FromInt(NewNodeIndex) + ". Total: " + FString::FromInt(TotalNodeCount));
	return NewNodeIndex;
}

void AKnowledgeGraph::SyncGraphNodePositionsToDatabase()
{
	bool log = true;

	LogMessage("SyncGraphNodePositionsToDatabase called", log, 0, TEXT("SyncGraphNodePositionsToDatabase: "));
	
	// Create a JSON writer and JSON Array
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	Writer->WriteArrayStart();

	for (int32 i = 0; i < TotalNodeCount; i++)
	{
		auto NodeStringId = NodeIdToStringMap[i];
		
		Writer->WriteObjectStart();
		Writer->WriteValue("ID", NodeStringId);
		Writer->WriteObjectStart("unreal_engine_location_728");
		Writer->WriteValue("X", nodePositions[i].X);
		Writer->WriteValue("Y", nodePositions[i].Y);
		Writer->WriteValue("Z", nodePositions[i].Z);
		Writer->WriteObjectEnd();
		Writer->WriteObjectEnd();
	}

	Writer->WriteArrayEnd();
	Writer->Close();

	// Setup HTTP request
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	Request->SetVerb("POST");
	Request->SetURL(Config.GraphNodePositionSyncUrl);
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(OutputString);

	// Define response handler
	Request->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		if(bWasSuccessful)
		{
			LogMessageInternal("Successfully sent JSON to server.", true, 2);
			GLog->Log(Response->GetContentAsString());
		}
		else
		{
			LogMessageInternal("Failed to send JSON.", true, 2);
		}
	});

	Request->ProcessRequest();
}

void AKnowledgeGraph::CleanUpObjects()
{
	// For every node
	for (int32 i = 0; i < TotalNodeCount; i++)
	{
		// Remove the text render component
		if (Config.bUseTextRenderComponents)
		{
			if (GraphNodes[i].textComponent && GraphNodes[i].textComponent->IsRegistered())
			{
				GraphNodes[i].textComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
				GraphNodes[i].textComponent->UnregisterComponent();
				GraphNodes[i].textComponent->DestroyComponent();
				GraphNodes[i].textComponent = nullptr;
			}
		}
	}
	
	if (Config.bUseInstancedStaticMesh)
	{
		InstancedStaticMeshComponent->ClearInstances();
	}

	// For each link
	for (int32 i = 0; i < GraphLinks.Num(); i++)
	{
		if (GraphLinks[i].EdgeMeshComponent && GraphLinks[i].EdgeMeshComponent->IsRegistered())
		{
			GraphLinks[i].EdgeMeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			GraphLinks[i].EdgeMeshComponent->UnregisterComponent();
			GraphLinks[i].EdgeMeshComponent->DestroyComponent();
			GraphLinks[i].EdgeMeshComponent = nullptr;
		}
	}
}

void AKnowledgeGraph::ReloadTheWholeGraph()
{
	CleanUpObjects();
	
	bGraphRequesting = true;
	bGraphInitialized = false;
	bPredefinedPositionNeedsUpdate = true;
	Prepare();
}
