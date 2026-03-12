// GraphDataManager.cpp
// Implementation of graph data management

#include "GraphDataManager.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void UGraphDataManager::RequestGraphData(EGraphCreationMode Mode, int32 JsonFileIndex, const TMap<int32, FString>& FileIndexToPath)
{
	ClearAllData();

	switch (Mode)
	{
	case EGraphCreationMode::FromDatabase:
		LogMessage("Requesting graph from database via HTTP", 0);
		RequestFromDatabase();
		break;

	case EGraphCreationMode::FromJson:
		LogMessage("Loading graph from JSON file", 0);
		RequestFromJsonFile(JsonFileIndex, FileIndexToPath);
		break;

	case EGraphCreationMode::AutoGenerate:
		LogMessage("Auto-generate mode - no data to load", 0);
		// For auto-generate, we don't load data, just notify success
		OnGraphDataLoaded.Broadcast(true);
		break;
	}
}

void UGraphDataManager::RequestFromDatabase()
{
	// Reset retry count for new request
	HttpRetryCount = 0;
	PendingRequestMode = EGraphCreationMode::FromDatabase;
	
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb("GET");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetURL("http://localhost:5007/api/v0/return_all_nodes_and_their_connections_if_any");
	
	// Set timeout (30 seconds)
	HttpRequest->SetTimeout(30.0f);
	
	HttpRequest->OnProcessRequestComplete().BindUObject(
		this,
		&UGraphDataManager::OnDatabaseRequestComplete
	);
	
	if (!HttpRequest->ProcessRequest())
	{
		LogMessage("Failed to initiate HTTP request", 2);
		OnGraphDataLoaded.Broadcast(false);
		return;
	}
	
	LogMessage("HTTP request sent to database (attempt " + FString::FromInt(HttpRetryCount + 1) + "/" + FString::FromInt(MaxHttpRetries) + ")", 0);
}

void UGraphDataManager::RequestFromJsonFile(int32 FileIndex, const TMap<int32, FString>& FileIndexToPath)
{
	// Validate file index
	if (!FileIndexToPath.Contains(FileIndex))
	{
		LogMessage("Invalid file index: " + FString::FromInt(FileIndex) + ". Available indices: " + FString::FromInt(FileIndexToPath.Num()), 2);
		OnGraphDataLoaded.Broadcast(false);
		return;
	}

	const FString JsonFilePath = FPaths::ProjectContentDir() + "/data/state/" + FileIndexToPath[FileIndex];
	
	// Check if file exists
	if (!FPaths::FileExists(JsonFilePath))
	{
		LogMessage("JSON file does not exist: " + JsonFilePath, 2);
		OnGraphDataLoaded.Broadcast(false);
		return;
	}
	
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *JsonFilePath))
	{
		LogMessage("Failed to read JSON file: " + JsonFilePath + ". Check file permissions.", 2);
		OnGraphDataLoaded.Broadcast(false);
		return;
	}

	// Validate JSON is not empty
	if (JsonString.IsEmpty())
	{
		LogMessage("JSON file is empty: " + JsonFilePath, 2);
		OnGraphDataLoaded.Broadcast(false);
		return;
	}

	LogMessage("Loaded JSON file (" + FString::FromInt(JsonString.Len()) + " characters): " + JsonFilePath, 0);

	if (ParseJsonData(JsonString))
	{
		LogMessage("Successfully loaded graph from JSON file", 0);
		OnGraphDataLoaded.Broadcast(true);
	}
	else
	{
		LogMessage("Failed to parse JSON file. Check JSON syntax and structure.", 2);
		OnGraphDataLoaded.Broadcast(false);
	}
}

void UGraphDataManager::OnDatabaseRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	// Check for basic failure
	if (!bWasSuccessful || !Response.IsValid())
	{
		// Retry logic
		if (HttpRetryCount < MaxHttpRetries - 1)
		{
			HttpRetryCount++;
			LogMessage("Request failed, retrying in " + FString::SanitizeFloat(HttpRetryDelaySeconds) + " seconds... (attempt " + 
				FString::FromInt(HttpRetryCount + 1) + "/" + FString::FromInt(MaxHttpRetries) + ")", 1);
			
			// Schedule retry using timer
			if (GetWorld())
			{
				GetWorld()->GetTimerManager().SetTimer(
					RetryTimerHandle,
					this,
					&UGraphDataManager::RetryDatabaseRequest,
					HttpRetryDelaySeconds,
					false
				);
			}
			return;
		}
		else
		{
			LogMessage("Request failed after " + FString::FromInt(MaxHttpRetries) + " attempts. Giving up.", 2);
			HandleRequestError(Request, Response);
			OnGraphDataLoaded.Broadcast(false);
			return;
		}
	}

	// Check HTTP status code
	const int32 ResponseCode = Response->GetResponseCode();
	if (ResponseCode < 200 || ResponseCode >= 300)
	{
		LogMessage("HTTP request returned error code: " + FString::FromInt(ResponseCode) + " - " + GetHttpErrorDescription(ResponseCode), 2);
		HandleRequestError(Request, Response);
		OnGraphDataLoaded.Broadcast(false);
		return;
	}

	// Validate content type
	const FString ContentType = Response->GetContentType();
	if (!ContentType.Contains("application/json"))
	{
		LogMessage("Response was not in JSON format. Received: " + ContentType, 2);
		LogMessage("Response preview: " + Response->GetContentAsString().Left(200), 2);
		OnGraphDataLoaded.Broadcast(false);
		return;
	}

	// Validate response is not empty
	const FString ResponseContent = Response->GetContentAsString();
	if (ResponseContent.IsEmpty())
	{
		LogMessage("Received empty response from server", 2);
		OnGraphDataLoaded.Broadcast(false);
		return;
	}

	LogMessage("Received response (" + FString::FromInt(ResponseContent.Len()) + " characters)", 0);

	// Parse and validate JSON
	if (ParseJsonData(ResponseContent))
	{
		LogMessage("Successfully loaded graph from database", 0);
		HttpRetryCount = 0; // Reset retry count on success
		OnGraphDataLoaded.Broadcast(true);
	}
	else
	{
		LogMessage("Failed to parse JSON from database. Check server response format.", 2);
		OnGraphDataLoaded.Broadcast(false);
	}
}

bool UGraphDataManager::ParseJsonData(const FString& JsonString)
{
	// Validate input
	if (JsonString.IsEmpty())
	{
		LogMessage("Cannot parse empty JSON string", 2);
		return false;
	}

	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonString);
	
	if (!FJsonSerializer::Deserialize(JsonReader, CachedJsonObject))
	{
		LogMessage("JSON deserialization failed. Invalid JSON syntax.", 2);
		return false;
	}
	
	if (!CachedJsonObject.IsValid())
	{
		LogMessage("JSON object is invalid after deserialization", 2);
		return false;
	}

	// Validate JSON structure
	if (!ValidateJsonStructure(CachedJsonObject))
	{
		LogMessage("JSON structure validation failed", 2);
		return false;
	}

	// Extract data with validation
	ExtractNodesFromJson(CachedJsonObject);
	ExtractLinksFromJson(CachedJsonObject);
	ExtractNodeProperties(CachedJsonObject);
	BuildIdMappings();

	// Final validation
	if (Nodes.Num() == 0)
	{
		LogMessage("Warning: No nodes were extracted from JSON", 1);
	}

	LogMessage("Parsed JSON successfully: " + FString::FromInt(Nodes.Num()) + " nodes, " + FString::FromInt(Links.Num()) + " links", 0);
	return true;
}

void UGraphDataManager::ExtractNodesFromJson(const TSharedPtr<FJsonObject>& JsonObject)
{
	if (!JsonObject.IsValid())
	{
		LogMessage("Cannot extract nodes from invalid JSON object", 2);
		return;
	}

	if (!JsonObject->HasField("nodes"))
	{
		LogMessage("JSON does not contain 'nodes' field", 1);
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* JsonNodesPtr = nullptr;
	if (!JsonObject->TryGetArrayField("nodes", JsonNodesPtr) || !JsonNodesPtr)
	{
		LogMessage("Failed to get 'nodes' array from JSON", 2);
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>& JsonNodes = *JsonNodesPtr;
	Nodes.Reserve(JsonNodes.Num());

	int32 ValidNodeCount = 0;
	for (int32 i = 0; i < JsonNodes.Num(); i++)
	{
		if (!JsonNodes[i].IsValid())
		{
			LogMessage("Skipping invalid node at index " + FString::FromInt(i), 1);
			continue;
		}

		TSharedPtr<FJsonObject> NodeObj = JsonNodes[i]->AsObject();
		if (!NodeObj.IsValid())
		{
			LogMessage("Skipping node at index " + FString::FromInt(i) + " - not a valid object", 1);
			continue;
		}

		// Validate node data
		if (!ValidateNodeData(NodeObj, i))
		{
			LogMessage("Skipping node at index " + FString::FromInt(i) + " - validation failed", 1);
			continue;
		}

		FNodeData NodeData(i, "");

		// Try to get string ID (different field names for different sources)
		if (NodeObj->HasField("user_generate_id_7577777777"))
		{
			NodeData.StringId = NodeObj->GetStringField("user_generate_id_7577777777");
		}
		else if (NodeObj->HasField("id"))
		{
			NodeData.StringId = NodeObj->GetStringField("id");
		}
		else
		{
			LogMessage("Warning: Node at index " + FString::FromInt(i) + " has no ID field", 1);
			NodeData.StringId = "node_" + FString::FromInt(i); // Generate fallback ID
		}

		// Try to get name
		if (NodeObj->HasField("name"))
		{
			NodeData.Name = NodeObj->GetStringField("name");
			
			// Handle special case: remove "everythingallaccount" prefix
			FString Prefix(TEXT("everythingallaccount"));
			if (NodeData.Name.StartsWith(Prefix))
			{
				NodeData.Name = "e" + NodeData.Name.Mid(Prefix.Len());
			}
		}
		else
		{
			NodeData.Name = "Node " + FString::FromInt(i); // Fallback name
		}

		// Try to get predefined location
		if (NodeObj->HasField("ue_location_X") && 
			NodeObj->HasField("ue_location_Y") && 
			NodeObj->HasField("ue_location_Z"))
		{
			NodeData.Position = FVector(
				NodeObj->GetNumberField("ue_location_X"),
				NodeObj->GetNumberField("ue_location_Y"),
				NodeObj->GetNumberField("ue_location_Z")
			);
		}

		Nodes.Add(NodeData);
		ValidNodeCount++;
	}

	LogMessage("Extracted " + FString::FromInt(ValidNodeCount) + " valid nodes from " + FString::FromInt(JsonNodes.Num()) + " total", 0);
}

void UGraphDataManager::ExtractLinksFromJson(const TSharedPtr<FJsonObject>& JsonObject)
{
	if (!JsonObject->HasField("links"))
	{
		LogMessage("JSON does not contain 'links' field", 1);
		return;
	}

	TArray<TSharedPtr<FJsonValue>> JsonLinks = JsonObject->GetArrayField("links");
	Links.Reserve(JsonLinks.Num());

	for (const TSharedPtr<FJsonValue>& LinkValue : JsonLinks)
	{
		TSharedPtr<FJsonObject> LinkObj = LinkValue->AsObject();
		if (!LinkObj.IsValid())
			continue;

		FString SourceId = LinkObj->GetStringField("source");
		FString TargetId = LinkObj->GetStringField("target");

		// We'll resolve these to indices after building the ID mappings
		FLinkData LinkData;
		Links.Add(LinkData);
		
		// Store temporarily - will be resolved in BuildIdMappings
		// For now, store as negative values to indicate they need resolution
		Links.Last().SourceIndex = -1;
		Links.Last().TargetIndex = -1;
	}

	LogMessage("Extracted " + FString::FromInt(Links.Num()) + " links from JSON", 0);
}

void UGraphDataManager::ExtractNodeProperties(const TSharedPtr<FJsonObject>& JsonObject)
{
	if (!JsonObject->HasField("nodes"))
		return;

	TArray<TSharedPtr<FJsonValue>> JsonNodes = JsonObject->GetArrayField("nodes");

	for (int32 i = 0; i < JsonNodes.Num() && i < Nodes.Num(); i++)
	{
		TSharedPtr<FJsonObject> NodeObj = JsonNodes[i]->AsObject();
		if (!NodeObj.IsValid())
			continue;

		// Extract all properties as strings
		for (const auto& Pair : NodeObj->Values)
		{
			if (Pair.Value->Type == EJson::String)
			{
				Nodes[i].Properties.Add(Pair.Key, Pair.Value->AsString());
			}
			else if (Pair.Value->Type == EJson::Number)
			{
				Nodes[i].Properties.Add(Pair.Key, FString::SanitizeFloat(Pair.Value->AsNumber()));
			}
		}
	}
}

void UGraphDataManager::BuildIdMappings()
{
	StringIdToIndex.Empty();
	IndexToStringId.Empty();

	for (int32 i = 0; i < Nodes.Num(); i++)
	{
		if (!Nodes[i].StringId.IsEmpty())
		{
			StringIdToIndex.Add(Nodes[i].StringId, i);
			IndexToStringId.Add(i, Nodes[i].StringId);
		}
	}

	// Now resolve link indices
	if (CachedJsonObject.IsValid() && CachedJsonObject->HasField("links"))
	{
		TArray<TSharedPtr<FJsonValue>> JsonLinks = CachedJsonObject->GetArrayField("links");
		
		for (int32 i = 0; i < JsonLinks.Num() && i < Links.Num(); i++)
		{
			TSharedPtr<FJsonObject> LinkObj = JsonLinks[i]->AsObject();
			if (!LinkObj.IsValid())
				continue;

			FString SourceId = LinkObj->GetStringField("source");
			FString TargetId = LinkObj->GetStringField("target");

			if (StringIdToIndex.Contains(SourceId) && StringIdToIndex.Contains(TargetId))
			{
				Links[i].SourceIndex = StringIdToIndex[SourceId];
				Links[i].TargetIndex = StringIdToIndex[TargetId];
			}
		}
	}
}

void UGraphDataManager::AddNodeToDatabase(const FString& NodeName, const FVector& Location)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL("http://localhost:3062/api/v0/create_node77777777");
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", "application/json");

	// Create JSON object
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("name", NodeName);
	JsonObject->SetNumberField("locationX", Location.X);
	JsonObject->SetNumberField("locationY", Location.Y);
	JsonObject->SetNumberField("locationZ", Location.Z);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	HttpRequest->SetContentAsString(RequestBody);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGraphDataManager::OnAddNodeRequestComplete);
	HttpRequest->ProcessRequest();

	LogMessage("Sending add node request to database", 0);
}

void UGraphDataManager::OnAddNodeRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			FString NewId = JsonObject->GetStringField("id");
			FString NewName = JsonObject->GetStringField("name");
			LogMessage("Node added successfully with ID: " + NewId, 0);
			
			// TODO: Notify listeners that a node was added
		}
		else
		{
			LogMessage("Failed to parse add node response", 2);
		}
	}
	else
	{
		HandleRequestError(Request, Response);
	}
}

void UGraphDataManager::UpdateAllNodePositionsToDatabase(const TArray<FVector>& Positions)
{
	if (Positions.Num() != Nodes.Num())
	{
		LogMessage("Position array size mismatch", 2);
		return;
	}

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	Writer->WriteArrayStart();

	for (int32 i = 0; i < Nodes.Num(); i++)
	{
		Writer->WriteObjectStart();
		Writer->WriteValue("ID", Nodes[i].StringId);
		Writer->WriteObjectStart("unreal_engine_location_728");
		Writer->WriteValue("X", Positions[i].X);
		Writer->WriteValue("Y", Positions[i].Y);
		Writer->WriteValue("Z", Positions[i].Z);
		Writer->WriteObjectEnd();
		Writer->WriteObjectEnd();
	}

	Writer->WriteArrayEnd();
	Writer->Close();

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetVerb("POST");
	Request->SetURL("http://localhost:3062/api/v0/update_position_of_all_nodes111");
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(OutputString);

	Request->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
	{
		if (bSuccess)
		{
			LogMessage("Successfully updated node positions to database", 0);
		}
		else
		{
			LogMessage("Failed to update node positions", 2);
		}
	});

	Request->ProcessRequest();
}

void UGraphDataManager::DeleteNodeFromDatabase(int32 NodeIndex)
{
	// TODO: Implement when backend endpoint is ready
	LogMessage("DeleteNodeFromDatabase not yet implemented", 1);
}

void UGraphDataManager::AddLinkToDatabase(int32 SourceIndex, int32 TargetIndex)
{
	// TODO: Implement when backend endpoint is ready
	LogMessage("AddLinkToDatabase not yet implemented", 1);
}

void UGraphDataManager::DeleteLinkFromDatabase(int32 LinkIndex)
{
	// TODO: Implement when backend endpoint is ready
	LogMessage("DeleteLinkFromDatabase not yet implemented", 1);
}

int32 UGraphDataManager::GetNodeIndexFromStringId(const FString& StringId) const
{
	const int32* IndexPtr = StringIdToIndex.Find(StringId);
	return IndexPtr ? *IndexPtr : -1;
}

FString UGraphDataManager::GetStringIdFromNodeIndex(int32 Index) const
{
	const FString* StringIdPtr = IndexToStringId.Find(Index);
	return StringIdPtr ? *StringIdPtr : FString();
}

const TMap<FString, FString>* UGraphDataManager::GetNodeProperties(int32 NodeIndex) const
{
	if (Nodes.IsValidIndex(NodeIndex))
	{
		return &Nodes[NodeIndex].Properties;
	}
	return nullptr;
}

void UGraphDataManager::ClearAllData()
{
	Nodes.Empty();
	Links.Empty();
	StringIdToIndex.Empty();
	IndexToStringId.Empty();
	CachedJsonObject.Reset();
}

void UGraphDataManager::HandleRequestError(FHttpRequestPtr Request, FHttpResponsePtr Response)
{
	LogMessage("HTTP Request failed", 2);

	if (!Response.IsValid())
	{
		LogMessage("No response received - possible causes:", 3);
		LogMessage("  - Server is not running", 3);
		LogMessage("  - Network connectivity issue", 3);
		LogMessage("  - Firewall blocking connection", 3);
		LogMessage("  - Request timeout", 3);
	}
	else
	{
		const int32 ResponseCode = Response->GetResponseCode();
		LogMessage("HTTP Status Code: " + FString::FromInt(ResponseCode) + " - " + GetHttpErrorDescription(ResponseCode), 3);
		
		const FString ResponseContent = Response->GetContentAsString();
		if (!ResponseContent.IsEmpty())
		{
			LogMessage("Response Content (first 500 chars): " + ResponseContent.Left(500), 3);
		}
		else
		{
			LogMessage("Response body is empty", 3);
		}
	}

	if (Request.IsValid())
	{
		LogMessage("Request Details:", 3);
		LogMessage("  HTTP Verb: " + Request->GetVerb(), 3);
		LogMessage("  URL: " + Request->GetURL(), 3);
		
		const TArray<FString> Headers = Request->GetAllHeaders();
		if (Headers.Num() > 0)
		{
			LogMessage("  Headers: " + FString::FromInt(Headers.Num()) + " total", 3);
		}
	}
}

void UGraphDataManager::RetryDatabaseRequest()
{
	LogMessage("Retrying database request...", 0);
	RequestFromDatabase();
}

bool UGraphDataManager::ValidateJsonStructure(const TSharedPtr<FJsonObject>& JsonObject) const
{
	if (!JsonObject.IsValid())
	{
		LogMessage("JSON object is null", 2);
		return false;
	}

	// Check for required fields
	if (!JsonObject->HasField("nodes"))
	{
		LogMessage("JSON missing required field: 'nodes'", 2);
		return false;
	}

	// Validate nodes is an array
	const TArray<TSharedPtr<FJsonValue>>* NodesArray = nullptr;
	if (!JsonObject->TryGetArrayField("nodes", NodesArray))
	{
		LogMessage("'nodes' field is not an array", 2);
		return false;
	}

	// Links field is optional, but if present, must be an array
	if (JsonObject->HasField("links"))
	{
		const TArray<TSharedPtr<FJsonValue>>* LinksArray = nullptr;
		if (!JsonObject->TryGetArrayField("links", LinksArray))
		{
			LogMessage("'links' field exists but is not an array", 1);
		}
	}

	return true;
}

bool UGraphDataManager::ValidateNodeData(const TSharedPtr<FJsonObject>& NodeObj, int32 NodeIndex) const
{
	if (!NodeObj.IsValid())
	{
		return false;
	}

	// Node should have at least an ID or name
	const bool bHasId = NodeObj->HasField("id") || NodeObj->HasField("user_generate_id_7577777777");
	const bool bHasName = NodeObj->HasField("name");

	if (!bHasId && !bHasName)
	{
		LogMessage("Node at index " + FString::FromInt(NodeIndex) + " has neither ID nor name", 1);
		return false;
	}

	return true;
}

bool UGraphDataManager::ValidateLinkData(const TSharedPtr<FJsonObject>& LinkObj, int32 LinkIndex) const
{
	if (!LinkObj.IsValid())
	{
		return false;
	}

	// Link must have source and target
	if (!LinkObj->HasField("source") || !LinkObj->HasField("target"))
	{
		LogMessage("Link at index " + FString::FromInt(LinkIndex) + " missing source or target", 1);
		return false;
	}

	return true;
}

FString UGraphDataManager::GetHttpErrorDescription(int32 ResponseCode) const
{
	switch (ResponseCode)
	{
	case -1:
		return "Network Error (connection failed or timeout)";
	case 400:
		return "Bad Request (invalid request format)";
	case 401:
		return "Unauthorized (authentication required)";
	case 403:
		return "Forbidden (access denied)";
	case 404:
		return "Not Found (endpoint does not exist)";
	case 500:
		return "Internal Server Error";
	case 502:
		return "Bad Gateway (server is down or unreachable)";
	case 503:
		return "Service Unavailable (server is overloaded)";
	case 504:
		return "Gateway Timeout (server took too long to respond)";
	default:
		if (ResponseCode >= 200 && ResponseCode < 300)
			return "Success";
		else if (ResponseCode >= 300 && ResponseCode < 400)
			return "Redirection";
		else if (ResponseCode >= 400 && ResponseCode < 500)
			return "Client Error";
		else if (ResponseCode >= 500)
			return "Server Error";
		else
			return "Unknown Status";
	}
}

void UGraphDataManager::LogMessage(const FString& Message, int32 Severity) const
{
	// For now, just use UE_LOG
	// In the future, this could integrate with the existing logging system
	switch (Severity)
	{
	case 0: // Info
		UE_LOG(LogTemp, Log, TEXT("[GraphDataManager] %s"), *Message);
		break;
	case 1: // Warning
		UE_LOG(LogTemp, Warning, TEXT("[GraphDataManager] %s"), *Message);
		break;
	case 2: // Error
		UE_LOG(LogTemp, Error, TEXT("[GraphDataManager] %s"), *Message);
		break;
	case 3: // Critical
		UE_LOG(LogTemp, Fatal, TEXT("[GraphDataManager] %s"), *Message);
		break;
	}
}
