// KnowledgeGraph_BlueprintAPI.cpp
//
// Blueprint-callable functions for user interaction and database operations.
// Provides the interface between Blueprints and the graph system for:
// - Node selection and manipulation
// - Database CRUD operations (Create, Read, Update, Delete)
// - Graph reloading and cleanup
//
// Key Functions:
// - select_closest_node_from_player222(): Find nearest node to player
// - add_node_to_database1115(): Create new node via HTTP
// - update_position_of_all_nodes_to_database1113(): Sync positions to database
// - reload_the_whole_graph(): Full graph refresh
//
// Part of the KnowledgeGraph refactoring - extracted from KnowledgeGraph5.cpp

#include "KnowledgeGraph.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

void AKnowledgeGraph::select_closest_node_from_player222()
{
	FVector player_location = get_player_location727();
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
		lp("The closest node index is: " + FString::FromInt(closest_node_index) + 
		   " with a distance of: " + FString::SanitizeFloat(min_distance), true, 2);
	}
	else
	{
		lp("Could not find the closest node.", true, 2);
	}
}

void AKnowledgeGraph::add_node_to_database1115(FString NodeName)
{
	FVector player_location = get_player_location727();
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL("http://localhost:3062/api/v0/create_node77777777");
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
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &AKnowledgeGraph::add_node_to_database1115httpCompleted);
	HttpRequest->ProcessRequest();
}

void AKnowledgeGraph::add_node_to_database1115httpCompleted(
	FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if(bWasSuccessful && Response->GetResponseCode() == 200)
	{
		lp("print_out_everything: " + Response->GetContentAsString(), true, 2);

		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			FString id = JsonObject->GetStringField("id");
			lp("id7: " + id, true, 2);
			FString name = JsonObject->GetStringField("name");
			FVector player_location = get_player_location727();
			late_add_node(name, id, player_location);
		}
		else
		{
			lp("Failed to parse JSON response", true, 2);
		}
	}
	else
	{
		FString ErrorInfo = Response.IsValid() ? Response->GetContentAsString() : TEXT("Unable to get any response");
		UE_LOG(LogTemp, Error, TEXT("Error adding node: %s"), *ErrorInfo);
	}
}

void AKnowledgeGraph::delete_node_from_database1116()
{
	// TODO: Implement node deletion
}

void AKnowledgeGraph::delete_link_from_database1117()
{
	// TODO: Implement link deletion
}

void AKnowledgeGraph::add_link_to_database1114()
{
	// TODO: Implement link addition
}

void AKnowledgeGraph::update_position_of_all_nodes_to_database1113()
{
	bool log = true;

	ll("222222222update_position_of_all_nodes_to_database1113 called", log, 0, TEXT("update_position_of_all_nodes_to_database1113: "));
	
	// Create a JSON writer and JSON Array
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	Writer->WriteArrayStart();

	for (int32 i = 0; i < jnodessss; i++)
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
	Request->SetURL("http://localhost:3062/api/v0/update_position_of_all_nodes111");
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(OutputString);

	// Define response handler
	Request->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		if(bWasSuccessful)
		{
			ll2("Successfully sent JSON to server.", true, 2);
			GLog->Log(Response->GetContentAsString());
		}
		else
		{
			ll2("Failed to send JSON.", true, 2);
		}
	});

	Request->ProcessRequest();
}

void AKnowledgeGraph::clean_up_objects()
{
	// For every node
	for (int32 i = 0; i < jnodessss; i++)
	{
		// Remove the text render component
		if (node_use_text_render_components)
		{
			if (all_nodes2[i].textComponent && all_nodes2[i].textComponent->IsRegistered())
			{
				all_nodes2[i].textComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
				all_nodes2[i].textComponent->UnregisterComponent();
				all_nodes2[i].textComponent->DestroyComponent();
				all_nodes2[i].textComponent = nullptr;
			}
		}
	}
	
	if (node_use_instance_static_mesh)
	{
		InstancedStaticMeshComponent->ClearInstances();
	}

	// For each link
	for (int32 i = 0; i < all_links2.Num(); i++)
	{
		if (all_links2[i].edgeMesh && all_links2[i].edgeMesh->IsRegistered())
		{
			all_links2[i].edgeMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			all_links2[i].edgeMesh->UnregisterComponent();
			all_links2[i].edgeMesh->DestroyComponent();
			all_links2[i].edgeMesh = nullptr;
		}
	}
}

void AKnowledgeGraph::reload_the_whole_graph()
{
	clean_up_objects();
	
	graph_requesting = true;
	graph_initialized = false;
	use_predefined_position_should_update_once = true;
	prepare();
}

void AKnowledgeGraph::late_add_node(FString NodeName, FString id, FVector location)
{
	if (refresh_whole_graph_again_after_editing)
	{
		reload_the_whole_graph();
	}
	else
	{
		// TODO: Implement incremental node addition without full reload
	}
}
