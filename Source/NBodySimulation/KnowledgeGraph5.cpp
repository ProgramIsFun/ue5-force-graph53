
#include "KnowledgeGraph.h"


#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"



void AKnowledgeGraph::request_a_graph()
{
	if (cgm == CGM::DATABASE)
	{
		request_graph_http();
	}
	else
	{

		if (cgm==CGM::JSON)
		{
				ll("hiiiiiiiii", true, 0, TEXT("YourFunction: "));
				const FString JsonFilePath = FPaths::ProjectContentDir() + "/data/state/" + fileIndexToPath[
					use_json_file_index];
				FString JsonString; //Json converted to FString
				FFileHelper::LoadFileToString(JsonString, *JsonFilePath);
				JsonObject1 = MakeShareable(new FJsonObject());
				TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

				if (
					FJsonSerializer::Deserialize(JsonReader, JsonObject1) &&
					JsonObject1.IsValid())
				{
					TArray<TSharedPtr<FJsonValue>> jnodes = JsonObject1->GetArrayField("nodes");
				}
				else
				{
					precheck_succeed = false;
					ll("Failed to deserialize JSON. ", true, 2);
					return;
				}
			
		}
		default_generate_graph_method();
	}
}

void AKnowledgeGraph::request_graph_http()
{
	TSharedPtr<FJsonObject> Js = MakeShareable(new FJsonObject());
	Js->SetStringField("some_field", "some_value");
	FString OutputString;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Js.ToSharedRef(), JsonWriter);
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	// HttpRequest->SetVerb("POST");
	HttpRequest->SetVerb("GET");
	HttpRequest->SetHeader("Content-Type", "application/json");
	// FString URL = "https://www.space-track.org";
	// https://jsonplaceholder.typicode.com/todos/1
	// HttpRequest->SetURL("https://jsonplaceholder.typicode.com/todos/1");
	// HttpRequest->SetURL("localhost:3062/api/v0/return_all_nodes111");

	HttpRequest->SetURL("http://localhost:3062/api/v0/return_all_nodes111");
	// HttpRequest->SetContentAsString(OutputString)
	HttpRequest->OnProcessRequestComplete().BindUObject(
		this,
		&AKnowledgeGraph::request_graph_httpCompleted
	);
	HttpRequest->ProcessRequest();
	ll("YourFunction called", true, 0, TEXT("YourFunction: "));
}
void AKnowledgeGraph::request_graph_httpCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	bool log = true;
	ll("request_graph_httpCompleted called", log, 0, TEXT("request_graph_httpCompleted: "));
	if (bWasSuccessful)
	{
		ll("Request was successful", log, 0, TEXT("request_graph_httpCompleted: "));
		if (
			// Content-Type: application/json; charset=utf-8
			Response->GetContentType() == "application/json" ||
			Response->GetContentType() == "application/json; charset=utf-8"
		)
		{
			JsonObject1 = MakeShareable(new FJsonObject());

			TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(
				Response->GetContentAsString());

			if (FJsonSerializer::Deserialize(JsonReader, JsonObject1) &&
				JsonObject1.IsValid())
			{
				ll("Successfully parsed JSON.", log, 0, TEXT("request_graph_httpCompleted: "));


				default_generate_graph_method();
			}
			else
			{
				ll("Failed to parse JSON.", true, 2);
			}
		}
		else
			
		{
			ll("Response was not in JSON format.", true, 2);
			ll(FString::Printf(TEXT("Received Content-Type: %s"), *Response->GetContentType()), true, 2);
		}
	}
	else
	{
		debug_error_request(Request, Response);
	}
}



void AKnowledgeGraph::add_node_to_database1115(FString NodeName)
{
	// Get the first character locations.
	FVector player_location =get_player_location727();
	
	// Create a JSON writer and JSON Array
	
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

	// Define response callback
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &AKnowledgeGraph::add_node_to_database1115httpCompleted);

	// Execute the request
	HttpRequest->ProcessRequest();

}

void AKnowledgeGraph::add_node_to_database1115httpCompleted(
FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful
	)
{
	if(bWasSuccessful && Response->GetResponseCode() == 200)
	{
		// Log the raw response content
		lp("print_out_everything: " + Response->GetContentAsString(), true, 2);
		// UE_LOG(LogTemp, Log, TEXT("Response: %s"), *Response->GetContentAsString());

		// Parse the JSON response
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			FString id = JsonObject->GetStringField("id");
			lp("id7: " + id, true, 2);
			FString name = JsonObject->GetStringField("name");
			FVector player_location =get_player_location727();
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

void AKnowledgeGraph::late_add_node(FString NodeName, FString id, FVector location)
{
	bool request_the_whole_graph_again = false;
	if (request_the_whole_graph_again)
	{
		use_predefined_position_should_update_once = true;

		prepare();
	}
	else
	{
	
		// Update the arrays with the Correct length


		// Create new objects.
		
	}




	
}

void AKnowledgeGraph::select_closest_node_from_player222()
{

	FVector player_location = get_player_location727();  // Assuming GetPlayerLocation() is the correct function to obtain player position.
	float min_distance = FLT_MAX;  // Use max float value for initial comparison.
	int32 closest_node_index = -1;    // To store the index of the closest node.

	// Check if nodePositions array is filled correctly with FVector elements
	if (nodePositions.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("nodePositions array is empty!"));
		return;
	}

	for (int i = 0; i < nodePositions.Num(); i++)
	{
		float distance = FVector::Dist(player_location, nodePositions[i]); // Calculate Euclidean distance between each node and the player.

		if (distance < min_distance)
		{
			min_distance = distance;  // Update min_distance with the smallest found distance.
			closest_node_index = i;   // Update the index of the closest node.
		}
	}

	// Logging the result. You can implement other actions depending on your use case.
	if (closest_node_index != -1)
	{
		selected_node_index = closest_node_index;

		selected_node_name = id_to_string[closest_node_index];
		lp("The closest node index is: " + FString::FromInt(closest_node_index) + " with a distance of: " + FString::SanitizeFloat(min_distance), true, 2);
		// UE_LOG(LogTemp, Log, TEXT("The closest node index is: %d with a distance of: %.2f"), closest_node_index, min_distance);
	}
	else
	{
		lp("Could not find the closest node.", true, 2);
		// UE_LOG(LogTemp, Warning, TEXT("Could not find the closest node."));
	}

}


void AKnowledgeGraph::delete_node_from_database1116()
{
}

void AKnowledgeGraph::delete_link_from_database1117()
{
}

void AKnowledgeGraph::add_link_to_database1114()
{
}

void AKnowledgeGraph::update_position_of_all_nodes_to_database1113()
{


	bool log = true;

	ll("222222222update_position_of_all_nodes_to_database1113 called", log, 0, TEXT("update_position_of_all_nodes_to_database1113: "));
	// Create a JSON writer and JSON Array
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	Writer->WriteArrayStart();

	// id_to_string
	for (int32 i = 0; i < jnodessss; i++)
	{
		auto string_id= id_to_string[i];
		
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
	Request->SetURL("http://localhost:3062/api/v0/update_position_of_all_nodes111"); //Your server's endpoint
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(OutputString);

	// Define response handler
	Request->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		if(bWasSuccessful)
		{
			// Handle your server's response here
			// GLog->Log("Successfully sent JSON to server.");
			ll2("Successfully sent JSON to server.", true, 2);
			GLog->Log(Response->GetContentAsString());
		}
		else
		{
			// GLog->Log("Failed to send JSON.");
			ll2("Failed to send JSON.", true, 2);
		}
	});

	// Send the request
	Request->ProcessRequest();
}


void AKnowledgeGraph::debug_error_request(FHttpRequestPtr Request, FHttpResponsePtr Response)
{
	precheck_succeed = false;
	ll("Request failed", true, 2);

	// Additional debugging information
	if (!Response.IsValid())
	{
		ll("No response was received.", true, 3);
	}
	else
	{
		ll(FString::Printf(TEXT("HTTP Status Code: %d"), Response->GetResponseCode()), true, 3);
		ll(FString::Printf(TEXT("Response Content: %s"), *Response->GetContentAsString()), true, 3);
	}

	
	// Log information about the request itself
	ll(FString::Printf(TEXT("HTTP Verb: %s"), *Request->GetVerb()), true, 3);
	ll(FString::Printf(TEXT("Requested URL: %s"), *Request->GetURL()), true, 3);

	// Checking connectivity or endpoint issues
	if (Response->GetResponseCode() == -1)
	{
		ll("Could be a network connectivity issue or the endpoint might be down.", true, 3);
	}
}
