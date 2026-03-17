// KnowledgeGraph_BlueprintAPI.cpp
//
// Blueprint-callable functions for user interaction and database operations.
// Provides the interface between Blueprints and the graph system for:
// - Node selection and manipulation
// - Database CRUD operations (Create, Read, Update, Delete)
// - Graph reloading and cleanup
//
// Key Functions:
// - SelectClosestGraphNodeToPlayer(): Find nearest node to player
// - AddGraphNodeToDatabase(): Create new node via HTTP
// - SyncGraphNodePositionsToDatabase(): Sync positions to database
// - ReloadTheWholeGraph(): Full graph refresh
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
		selected_node_index = closest_node_index;
		selected_node_name = id_to_string[closest_node_index];
		LogToScreen("The closest node index is: " + FString::FromInt(closest_node_index) + 
		   " with a distance of: " + FString::SanitizeFloat(min_distance), true, 2);
	}
	else
	{
		LogToScreen("Could not find the closest node.", true, 2);
	}
}

void AKnowledgeGraph::AddGraphNodeToDatabase(FString NodeName)
{
	FVector player_location = GetPlayerLocation();
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(Config.GraphNodeCreateUrl);
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", "application/json");

	// Create JSON Object
	TSharedPtr<FJsonObject> json_object_772 = MakeShareable(new FJsonObject);
	json_object_772->SetStringField("name", NodeName);
	json_object_772->SetNumberField("locationX", player_location.X);
	json_object_772->SetNumberField("locationY", player_location.Y);
	json_object_772->SetNumberField("locationZ", player_location.Z);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(json_object_772.ToSharedRef(), Writer);

	HttpRequest->SetContentAsString(RequestBody);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &AKnowledgeGraph::OnAddGraphNodeHttpCompleted);
	HttpRequest->ProcessRequest();
}

void AKnowledgeGraph::OnAddGraphNodeHttpCompleted(
	FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if(bWasSuccessful && Response->GetResponseCode() == 200)
	{
		LogToScreen("print_out_everything: " + Response->GetContentAsString(), true, 2);

		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			FString id = JsonObject->GetStringField("id");
			LogToScreen("id7: " + id, true, 2);
			FString name = JsonObject->GetStringField("name");
			FVector player_location = GetPlayerLocation();
			late_add_node(name, id, player_location);
		}
		else
		{
			LogToScreen("Failed to parse JSON response", true, 2);
		}
	}
	else
	{
		FString ErrorInfo = Response.IsValid() ? Response->GetContentAsString() : TEXT("Unable to get any response");
		UE_LOG(LogTemp, Error, TEXT("Error adding node: %s"), *ErrorInfo);
	}
}

void AKnowledgeGraph::DeleteGraphNodeFromDatabase()
{
	// TODO: Implement node deletion
}

void AKnowledgeGraph::DeleteGraphLinkFromDatabase()
{
	// TODO: Implement link deletion
}

void AKnowledgeGraph::AddGraphLinkToDatabase()
{
	// TODO: Implement link addition
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
		auto string_id = id_to_string[i];
		
		Writer->WriteObjectStart();
		Writer->WriteValue("ID", string_id);
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
	
	graph_requesting = true;
	graph_initialized = false;
	use_predefined_position_should_update_once = true;
	prepare();
}

void AKnowledgeGraph::late_add_node(FString NodeName, FString id, FVector location)
{
	if (refresh_whole_graph_again_after_editing)
	{
		ReloadTheWholeGraph();
	}
	else
	{
		// TODO: Implement incremental node addition without full reload
	}
}
