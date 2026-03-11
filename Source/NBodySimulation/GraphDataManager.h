// GraphDataManager.h
// Handles all data operations: HTTP requests, JSON parsing, node/link storage

#pragma once


#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "GraphConfiguration.h"
#include "GraphDataManager.generated.h"

// Forward declarations
class AKnowledgeNode;
class UTextRenderComponent;

// Simple data structures for nodes and links
USTRUCT(BlueprintType)
struct FNodeData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Id = -1;

	UPROPERTY()
	FString StringId;

	UPROPERTY()
	FString Name;

	UPROPERTY()
	FVector Position = FVector::ZeroVector;

	UPROPERTY()
	TMap<FString, FString> Properties;

	// Runtime references (not serialized)
	AKnowledgeNode* NodeActor = nullptr;
	UTextRenderComponent* TextComponent = nullptr;

	FNodeData() {}
	FNodeData(int32 InId, const FString& InStringId) : Id(InId), StringId(InStringId) {}
};

USTRUCT(BlueprintType)
struct FLinkData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 SourceIndex = -1;

	UPROPERTY()
	int32 TargetIndex = -1;

	UPROPERTY()
	float Bias = 0.5f;

	UPROPERTY()
	float Strength = 1.0f;

	UPROPERTY()
	float Distance = 30.0f;

	// Runtime references (not serialized)
	class AKnowledgeEdge* EdgeActor = nullptr;
	class UStaticMeshComponent* EdgeMesh = nullptr;

	FLinkData() {}
	FLinkData(int32 Source, int32 Target) : SourceIndex(Source), TargetIndex(Target) {}
};

// Delegate for when graph data is loaded
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGraphDataLoaded, bool, bSuccess);

/**
 * Manages all graph data operations including HTTP requests, JSON parsing, and data storage
 */
UCLASS()
class NBODYSIMULATION_API UGraphDataManager : public UObject
{
	GENERATED_BODY()

public:
	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Graph Data")
	FOnGraphDataLoaded OnGraphDataLoaded;

	// Main API
	void RequestGraphData(EGraphCreationMode Mode, int32 JsonFileIndex, const TMap<int32, FString>& FileIndexToPath);
	void AddNodeToDatabase(const FString& NodeName, const FVector& Location);
	void DeleteNodeFromDatabase(int32 NodeIndex);
	void UpdateAllNodePositionsToDatabase(const TArray<FVector>& Positions);
	void AddLinkToDatabase(int32 SourceIndex, int32 TargetIndex);
	void DeleteLinkFromDatabase(int32 LinkIndex);

	// Data access
	const TArray<FNodeData>& GetNodes() const { return Nodes; }
	const TArray<FLinkData>& GetLinks() const { return Links; }
	int32 GetNodeCount() const { return Nodes.Num(); }
	int32 GetLinkCount() const { return Links.Num(); }

	// ID mapping
	int32 GetNodeIndexFromStringId(const FString& StringId) const;
	FString GetStringIdFromNodeIndex(int32 Index) const;

	// Node properties
	const TMap<FString, FString>* GetNodeProperties(int32 NodeIndex) const;

	// Clear all data
	void ClearAllData();

private:
	// Data storage
	UPROPERTY()
	TArray<FNodeData> Nodes;

	UPROPERTY()
	TArray<FLinkData> Links;

	TMap<FString, int32> StringIdToIndex;
	TMap<int32, FString> IndexToStringId;

	// Cached JSON object from last request
	TSharedPtr<FJsonObject> CachedJsonObject;

	// Retry logic for HTTP requests
	int32 HttpRetryCount = 0;
	static constexpr int32 MaxHttpRetries = 3;
	static constexpr float HttpRetryDelaySeconds = 2.0f;
	FTimerHandle RetryTimerHandle;
	EGraphCreationMode PendingRequestMode = EGraphCreationMode::AutoGenerate;

	// HTTP request handlers
	void RequestFromDatabase();
	void RequestFromJsonFile(int32 FileIndex, const TMap<int32, FString>& FileIndexToPath);
	void OnDatabaseRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnAddNodeRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	
	// Retry logic
	void RetryDatabaseRequest();

	// JSON parsing
	bool ParseJsonData(const FString& JsonString);
	void ExtractNodesFromJson(const TSharedPtr<FJsonObject>& JsonObject);
	void ExtractLinksFromJson(const TSharedPtr<FJsonObject>& JsonObject);
	void ExtractNodeProperties(const TSharedPtr<FJsonObject>& JsonObject);
	
	// JSON validation
	bool ValidateJsonStructure(const TSharedPtr<FJsonObject>& JsonObject) const;
	bool ValidateNodeData(const TSharedPtr<FJsonObject>& NodeObj, int32 NodeIndex) const;
	bool ValidateLinkData(const TSharedPtr<FJsonObject>& LinkObj, int32 LinkIndex) const;

	// ID mapping helpers
	void BuildIdMappings();

	// Error handling
	void HandleRequestError(FHttpRequestPtr Request, FHttpResponsePtr Response);
	FString GetHttpErrorDescription(int32 ResponseCode) const;

	// Logging helper
	void LogMessage(const FString& Message, int32 Severity = 0) const;
};
