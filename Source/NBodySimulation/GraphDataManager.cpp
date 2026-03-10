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
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb("GET");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetURL("http://localhost:5007/api/v0/return_all_nodes_and_their_connections_if_any");
	
	HttpRequest->OnProcessRequestComplete().BindUObject(
		this,
		&UGraphDataManager::OnDatabaseRequestComplete
	);
	
	HttpRequest->ProcessRequest();
	LogMessage("HTTP request sent to database", 0);
}

void UGraphDataManager::RequestFromJsonFile(int32 FileIndex, const TMap<int32, FString>& FileIndexToPath)
{
	if (!FileIndexToPath.Contains(FileIndex))
	{
		LogMessage("Invalid file index: " + FString::FromInt(FileIndex), 2);
		OnGraphDataLoaded.Broadcast(false);
		return;
	}

	const FString JsonFilePath = FPaths::ProjectContentDir() + "/data/state/" + FileIndexToPath[FileIndex];
	FString JsonString;

	if (!FFileHelper::LoadFileToString(JsonString, *JsonFilePath))
	{
		LogMessage("Failed to load JSON file: " + JsonFilePath, 2);
		OnGraphDataLoaded.Broadcast(false);
		return;
	}

	if (ParseJsonData(JsonString))
	{
		LogMessage("Successfully loaded graph from JSON file", 0);
		OnGraphDataLoaded.Broadcast(true);
	}
	else
	{
		LogMessage("Failed to parse JSON file", 2);
		OnGraphDataLoaded.Broadcast(false);
	}
}

void UGraphDataManager::OnDatabaseRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		HandleRequestError(Request, Response);
		OnGraphDataLoaded.Broadcast(false);
		return;
	}

	const FString ContentType = Response->GetContentType();
	if (ContentType != "application/json" && ContentType != "application/json; charset=utf-8")
	{
		LogMessage("Response was not in JSON format. Received: " + ContentType, 2);
		OnGraphDataLoaded.Broadcast(false);
		return;
	}

	if (ParseJsonData(Response->GetContentAsString()))
	{
		LogMessage("Successfully loaded graph from database", 0);
		OnGraphDataLoaded.Broadcast(true);
	}
	else
	{
		LogMessage("Failed to parse JSON from database", 2);
		OnGraphDataLoaded.Broadcast(false);
	}
}

bool UGraphDataManager::ParseJsonData(const FString& JsonString)
{
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonString);
	
	if (!FJsonSerializer::Deserialize(JsonReader, CachedJsonObject) || !CachedJsonObject.IsValid())
	{
		return false;
	}

	ExtractNodesFromJson(CachedJsonObject);
	ExtractLinksFromJson(CachedJsonObject);
	ExtractNodeProperties(CachedJsonObject);
	BuildIdMappings();

	return true;
}

void UGraphDataManager::ExtractNodesFromJson(const TSharedPtr<FJsonObject>& JsonObject)
{
	if (!JsonObject->HasField("nodes"))
	{
		LogMessage("JSON does not contain 'nodes' field", 1);
		return;
	}

	TArray<TSharedPtr<FJsonValue>> JsonNodes = JsonObject->GetArrayField("nodes");
	Nodes.Reserve(JsonNodes.Num());

	for (int32 i = 0; i < JsonNodes.Num(); i++)
	{
		TSharedPtr<FJsonObject> NodeObj = JsonNodes[i]->AsObject();
		if (!NodeObj.IsValid())
			continue;

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
	}

	LogMessage("Extracted " + FString::FromInt(Nodes.Num()) + " nodes from JSON", 0);
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
		LogMessage("No response received", 3);
	}
	else
	{
		LogMessage("HTTP Status Code: " + FString::FromInt(Response->GetResponseCode()), 3);
		LogMessage("Response Content: " + Response->GetContentAsString(), 3);
	}

	if (Request.IsValid())
	{
		LogMessage("HTTP Verb: " + Request->GetVerb(), 3);
		LogMessage("Requested URL: " + Request->GetURL(), 3);
	}

	if (Response.IsValid() && Response->GetResponseCode() == -1)
	{
		LogMessage("Network connectivity issue or endpoint is down", 3);
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
