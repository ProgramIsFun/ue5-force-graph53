// KnowledgeGraph_DataIntegration.cpp
//
// Integration layer between KnowledgeGraph and GraphDataManager.
// Handles data loading from JSON files, HTTP endpoints, and auto-generation.
// Converts between new data structures (FNodeData, FLinkData) and legacy format.
//
// Key Functions:
// - OnGraphDataLoadedCallback(): Async callback when data is loaded
// - prepare(): Initiates graph data loading based on creation mode
// - generate_objects_for_node_and_link_new(): Creates visual objects from data
// - request_graph_http(): Legacy HTTP request functions (kept for compatibility)
//
// Part of the KnowledgeGraph refactoring - integrates GraphDataManager component

#include "KnowledgeGraph.h"
#include "NBodyUtils.h"

void AKnowledgeGraph::OnGraphDataLoadedCallback(bool bSuccess)
{
	ll("OnGraphDataLoadedCallback called with success: " + FString(bSuccess ? "true" : "false"), true, 0);

	if (!bSuccess)
	{
		ll("Failed to load graph data!", true, 2);
		precheck_succeed = false;
		return;
	}

	// Data is loaded, now convert it to the old format for compatibility
	// In the future, we'll refactor the rest of the code to use DataManager directly

	const TArray<FNodeData>& LoadedNodes = DataManager->GetNodes();
	const TArray<FLinkData>& LoadedLinks = DataManager->GetLinks();

	jnodessss = LoadedNodes.Num();
	ll("Loaded " + FString::FromInt(jnodessss) + " nodes from DataManager", true, 0);

	// Build ID mappings (for backward compatibility)
	id_to_string.Empty();
	string_to_id.Empty();
	for (int32 i = 0; i < LoadedNodes.Num(); i++)
	{
		if (!LoadedNodes[i].StringId.IsEmpty())
		{
			string_to_id.Add(LoadedNodes[i].StringId, i);
			id_to_string.Add(i, LoadedNodes[i].StringId);
		}
	}

	// Copy predefined positions if available
	if (Config.bUsePredefinedLocation)
	{
		predefined_positions.SetNum(jnodessss);
		for (int32 i = 0; i < jnodessss; i++)
		{
			predefined_positions[i] = LoadedNodes[i].Position;
		}

		// Center to current actor if needed
		if (Config.bCenterPredefinedLocationToActor)
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

	// Extract node properties (for backward compatibility)
	AllNodeProperties.Empty();
	for (const FNodeData& Node : LoadedNodes)
	{
		AllNodeProperties.Add(Node.Properties);
	}

	// Now continue with the old flow
	initialize_arrays();
	
	// Copy links to old format and create their meshes
	// LoadedLinks already declared at the top of the function
	all_links2.SetNum(LoadedLinks.Num());
	
	for (int32 i = 0; i < LoadedLinks.Num(); i++)
	{
		// Use add_edge to properly create the link with its mesh
		add_edge(i, LoadedLinks[i].SourceIndex, LoadedLinks[i].TargetIndex);
	}
	
	if (generate_objects_for_node_and_link_new())
	{
		return;
	}

	ll("post generate graph", true, 0);
	post_generate_graph();
}

// Updated prepare() function to use DataManager
void AKnowledgeGraph::prepare()
{
	ll("prepare() called - using new DataManager", true, 0);

	if (cgm == CGM::GENERATE)
	{
		// Auto-generate mode - don't load data, just set up arrays
		ll("Auto-generate mode", true, 0);
		jnodessss = Config.AutoGenerateNodeCount;
		
		initialize_arrays();
		miscellaneous(); // Creates the links
		
		if (generate_objects_for_node_and_link())
		{
			return;
		}

		ll("post generate graph", true, 0);
		post_generate_graph();
	}
	else
	{
		// Use DataManager to load from JSON or Database
		ll("Loading graph via DataManager", true, 0);
		
		// Convert old enum to new enum
		EGraphCreationMode Mode = EGraphCreationMode::AutoGenerate;
		if (cgm == CGM::JSON)
		{
			Mode = EGraphCreationMode::FromJson;
		}
		else if (cgm == CGM::DATABASE)
		{
			Mode = EGraphCreationMode::FromDatabase;
		}

		// Request data - callback will be triggered when done
		DataManager->RequestGraphData(Mode, Config.JsonFileIndex, fileIndexToPath);
	}

	if (Config.bUsePredefinedLocation)
	{
		// Predefined locations could be regarded as stable
		alpha = 0;
	}
}


// Updated generate_objects_for_node_and_link to work with DataManager
bool AKnowledgeGraph::generate_objects_for_node_and_link_new()
{
	bool log = true;
	
	if (cgm == CGM::GENERATE)
	{
		// Auto-generate mode - create simple text labels
		for (int32 i = 0; i < jnodessss; i++)
		{
			if (Config.bUseTextRenderComponents)
			{
				FString name = "Sample Text : " + FString::FromInt(i);
				generate_text_render_component_and_attach(name, i);
			}
		}
	}
	else
	{
		// Use DataManager data
		const TArray<FNodeData>& Nodes = DataManager->GetNodes();
		
		for (int32 i = 0; i < jnodessss; i++)
		{
			if (Config.bUseTextRenderComponents)
			{
				FString name = Nodes[i].Name;
				if (name.IsEmpty())
				{
					name = "Node " + FString::FromInt(i);
				}
				generate_text_render_component_and_attach(name, i);
			}
		}
		ll("Number of nodes generated: " + FString::FromInt(jnodessss), log);
		ll("Number of links: " + FString::FromInt(all_links2.Num()), log);
	}
	
	return false;
}

// Legacy HTTP request functions (kept for backward compatibility)
// These are now mostly handled by GraphDataManager, but kept for any direct usage

void AKnowledgeGraph::request_a_graph()
{
	if (cgm == CGM::DATABASE)
	{
		ll("cgm is database via HTTP. ", true, 0, TEXT("YourFunction: "));
		request_graph_http();
	}
	else
	{
		if (cgm==CGM::JSON)
		{
			ll("cgm is json", true, 0, TEXT("YourFunction: "));
			const FString JsonFilePath = FPaths::ProjectContentDir() + "/data/state/" + fileIndexToPath[
				Config.JsonFileIndex];
			FString JsonString;
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
		}else
		{
			ll("cgm is something else, should be auto generate. ", true, 0, TEXT("YourFunction: "));
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
	HttpRequest->SetVerb("GET");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetURL("http://localhost:5007/api/v0/return_all_nodes_and_their_connections_if_any");
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

void AKnowledgeGraph::debug_error_request(FHttpRequestPtr Request, FHttpResponsePtr Response)
{
	precheck_succeed = false;
	ll("Request failed", true, 2);

	if (!Response.IsValid())
	{
		ll("No response was received.", true, 3);
	}
	else
	{
		ll(FString::Printf(TEXT("HTTP Status Code: %d"), Response->GetResponseCode()), true, 3);
		ll(FString::Printf(TEXT("Response Content: %s"), *Response->GetContentAsString()), true, 3);
	}

	ll(FString::Printf(TEXT("HTTP Verb: %s"), *Request->GetVerb()), true, 3);
	ll(FString::Printf(TEXT("Requested URL: %s"), *Request->GetURL()), true, 3);

	if (Response->GetResponseCode() == -1)
	{
		ll("Could be a network connectivity issue or the endpoint might be down.", true, 3);
	}
}
