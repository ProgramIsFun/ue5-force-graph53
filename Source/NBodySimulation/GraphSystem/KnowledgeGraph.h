// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "CoreMinimal.h"
#include "NBodyOctree.h"
#include "NBodyUtils.h"

#include "NBodySimModule.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "GraphConfiguration.h"
#include "GraphDataManager.h"
#include "GraphPhysicsSimulator.h"
#include "GraphRenderer.h"


///////////////////////////////////////////////////////////////////////////////////////

#include "KnowledgeGraph.generated.h"

class GraphLink
{
public:
	int32 SourceNodeIndex;
	int32 TargetNodeIndex;
	float LinkBias;
	float LinkStrength;
	float LinkDistance;
	UStaticMeshComponent* EdgeMeshComponent = nullptr;
	GraphLink(int32 InSourceNodeIndex, int32 InTargetNodeIndex)
	{
		SourceNodeIndex = InSourceNodeIndex;
		TargetNodeIndex = InTargetNodeIndex;
	}
	GraphLink()
	{
	}
};

class GraphNode
{
public:
	int id;
	UTextRenderComponent* textComponent = nullptr;
	GraphNode(int InId, UTextRenderComponent* InTextComponent)
	{
		id = InId;
		textComponent = InTextComponent;
	}
	GraphNode()
		: id(-1)
	{
	}
};

UCLASS()
class NBODYSIMULATION_API AKnowledgeGraph : public AActor
{
	GENERATED_BODY()
	
public:

	// Blueprint-exposed graph text functions
	UFUNCTION(BlueprintCallable, Category = "Graph Text")
	void SetGraphNodeTextSize(float size);
	UFUNCTION(BlueprintCallable, Category = "Graph Text")
	void AdjustGraphNodeTextSize(bool increase, float size);
	UFUNCTION(BlueprintCallable, Category = "Graph Text")
	void IncreaseGraphNodeTextSize(float size);
	UFUNCTION(BlueprintCallable, Category = "Graph Text")
	void DecreaseGraphNodeTextSize(float size);

	UFUNCTION(BlueprintCallable, Category = "Graph Database")
	void SyncGraphNodePositionsToDatabase();
	UFUNCTION(BlueprintCallable, Category = "Graph Database")
	void AddGraphLinkToDatabase();
	UFUNCTION(BlueprintCallable, Category = "Graph Database")
	void AddGraphNodeToDatabase(FString NodeName);
	void OnAddGraphNodeHttpCompleted(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);
	void CleanUpObjects();
	void ReloadTheWholeGraph();
	void LateAddNode(FString NodeName, FString id, FVector location);

	UFUNCTION(BlueprintCallable, Category = "Graph Selection")
	void SelectClosestGraphNodeToPlayer();
	
	UFUNCTION(BlueprintCallable, Category = "Graph Database")
	void DeleteGraphNodeFromDatabase();
	UFUNCTION(BlueprintCallable, Category = "Graph Database")
	void DeleteGraphLinkFromDatabase();
	
	// Temporary variables.
	bool graph_requesting = false;
	bool graph_initialized = false;

	bool refresh_whole_graph_again_after_editing = true;
	
	bool use_predefined_position_should_update_once = true;
	bool precheck_succeed = true;

	// Server connection failure shutdown
	bool bServerConnectionFailed = false;
	float ServerConnectionShutdownTimer = 5.0f;
	FVector current_own_position;
	bool GPUvalid = false;
	

	
	// Important Variables.
	TSharedPtr<FJsonObject> JsonObject1;

	int32 selected_node_index = -1;
	int32 selected_node_index_previous = -1;
	FString selected_node_name = "";
	
	// Total number of nodes in the graph (formerly: jnodessss)
	int32 TotalNodeCount;
	
	TMap<int32, FString> id_to_string;
	TMap<FString, int32> string_to_id;
	
	// Array of all graph nodes (formerly: all_nodes2)
	TArray<GraphNode> GraphNodes;
	
	// Array of all graph links/edges (formerly: all_links2)
	TArray<GraphLink> GraphLinks;
	
	TArray<FVector> predefined_positions;

	
	TArray<FVector> nodePositions;
	TArray<FVector> nodeVelocities;







	
	UPROPERTY(VisibleAnywhere, Instanced)
	TObjectPtr<UInstancedStaticMeshComponent> InstancedStaticMeshComponent;

	UPROPERTY(VisibleAnywhere)
	TArray<UTextRenderComponent*> GraphNodeTextComponents;
	

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
	TMap<int32, FString> fileIndexToPath = {
		{0, "statered.json"},
		{1, "state - 2024-06-18T223257.374.json"},
		{2, "state777777777.json"},
	};



	// Internal functions. 
	void GpuGetPositions();
	bool MainFunction(float DeltaTime);
	void RotateToFacePlayer();
	void PassParametersToShaderManagement();
	void PostGenerateGraph();
	void Prepare();
	void UpdateParameterInShader(float DeltaTime);
	bool IsGraphStabilized(bool log);
	void CpuCalculate();
	void UpdatePositionArray(bool log);
	void UpdateAlpha();
	void UpdateIterations();
	void GenerateTextRenderComponentAndAttach(FString name,int32 index);
	void GetNumberOfNodes();
	void CreateOneToOneMapping();
	void Miscellaneous();
	void SetArrayLengths();
	void SetArrayValues();
	void InitializeArrays();
	bool GenerateObjectsForNodeAndLink();
	bool GenerateObjectsForNodeAndLinkNew(); // New version using DataManager
	void DealWithPredefinedLocation();
	void DefaultGenerateGraphMethod();
	void AddEdge(int32 id, int32 source, int32 target);
	void InitializeNodePosition();
	void InitializeNodePositionIndividual(int index);
	FVector GetPlayerLocation();
	FVector GetLocationInFrontOfPlayer();
	void CalculateBiasAndStrengthOfLinks();
	void ApplyForce();
	void CalculateLinkForceAndUpdateVelocity();
	void CalculateChargeForceAndUpdateVelocity();
	void CalculateCentreForceAndUpdatePosition();
	void UpdateNodeWorldPositionAccordingToPositionArray();
	void UpdatePositionArrayAccordingToVelocityArray();
	void UpdateLinkPosition();
	void RequestGraphHttpCompleted(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);
	void RequestGraphHttp();
	void RequestAGraph();
	void DebugErrorRequest(FHttpRequestPtr Request, FHttpResponsePtr Response);

	// New callback for DataManager
	UFUNCTION()
	void OnGraphDataLoadedCallback(bool bSuccess);

	// Physics simulator integration
	void InitializePhysicsSimulator();
	void CpuCalculateNew(); // New version using PhysicsSimulator

	// Renderer integration
	void UpdateNodeWorldPositionAccordingToPositionArrayNew();
	void UpdateLinkPositionNew();
	void RotateToFacePlayerNew();



	// New unified configuration struct
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graph Configuration")
	FGraphConfiguration Config;

	// New data manager - handles all HTTP/JSON/database operations
	UPROPERTY()
	UGraphDataManager* DataManager;

	// New physics simulator - handles all force calculations
	UPROPERTY()
	UGraphPhysicsSimulator* PhysicsSimulator;

	// New renderer - handles all visualization
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering")
	UGraphRenderer* Renderer;

	// Runtime iteration tracking (not in Config because it changes every frame)
	int32 FrameSkipCounter = 0;
	int32 SimulationIterationCount = 0;





	// Unreal engine actor functions. 
	AKnowledgeGraph();
	virtual ~AKnowledgeGraph() override;
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;







	// Debug visualization
	void DrawDebugGrid(const FVector& Center, int32 GridSize = 10, float CellSize = 100.0f);

	// Helper functions.
	template <typename Func, typename... Args>
	auto timeThisMemberFunction(const char* functionName, Func function, Args&&... args)
	{
		// auto start = std::chrono::high_resolution_clock::now();
		double StartTime = FPlatformTime::Seconds();
		(this->*function)(std::forward<Args>(args)...);
		double EndTime = FPlatformTime::Seconds();
		double ElapsedTime = EndTime - StartTime;
		LogAlways("Elapsed time For " + FString(functionName) + ": " + FString::SanitizeFloat(ElapsedTime) + " seconds");
		return ElapsedTime;
	}
	void QuitGame()
	{
		LogMessage("received QuitGame to quit the game", true, 2);
		// This doesn't block the game thread. It just sent a request to end the game. 
		UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit,
		                               false);
	}
	void StopRendering()
	{
		FNBodySimModule::Get().EndRendering();
	}
	void LogMessage(const FString& StringToLog, bool LOG=false, int SeverityLevel = 0, const FString& Prefix = TEXT("[Info]"))
	{
		if(Config.bEnableLogging)
		{
			LogMessageInternal(StringToLog, LOG, SeverityLevel, Prefix);
		}else
		{
			// Do nothing possibly for performance
		}
	}
	void LogToScreen(const FString& StringToLog, bool LOG=false, int SeverityLevel = 0, const FString& Prefix = TEXT("[Info]"))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, StringToLog);
		UE_LOG(LogTemp, Warning, TEXT("%s"), *StringToLog);
	}



	// Shader related variables. 
	FNBodySimParameters SimParameters;
	UPROPERTY()
	TArray<FTransform> BodyTransforms;
	TArray<int> LinkOffsets; // Holds the offset for each body
	TArray<int> LinkCounts; // Holds the count of links for each body
	TArray<int> LinkIndices; // Flat array containing all links
	TArray<float> LinkStrengths; // Holds the strength of each link
	TArray<float> LinkBiases; // Holds the bias of each link
	TArray<int> Linkinout;



	// Other variables. 
	OctreeNode* OctreeData2;



	
};


