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

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGraphNodeSelectionChanged, int32 /* SelectedNodeIndex */);

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
		: SourceNodeIndex(InSourceNodeIndex), TargetNodeIndex(InTargetNodeIndex),
		  LinkBias(0.5f), LinkStrength(1.0f), LinkDistance(30.0f)
	{
	}
	GraphLink()
		: SourceNodeIndex(-1), TargetNodeIndex(-1),
		  LinkBias(0.5f), LinkStrength(1.0f), LinkDistance(30.0f)
	{
	}
};

class GraphNode
{
public:
	UTextRenderComponent* textComponent = nullptr;
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

	UFUNCTION(BlueprintCallable, Category = "Graph Selection")
	void SelectClosestGraphNodeToPlayer();

	// Selects the node closest to the camera look-at ray (crosshair pick).
	// Highlights the selected node's text in yellow; deselects the previous one.
	UFUNCTION(BlueprintCallable, Category = "Graph Selection")
	void SelectGraphNodeByLookAt();

	// Laser-select: hold to aim, release to confirm selection.
	// Call BeginLaserSelect on key press, EndLaserSelect on key release.
	// DrawLaserSelectRay is called every tick while active.
	UFUNCTION(BlueprintCallable, Category = "Graph Selection")
	void BeginLaserSelect();
	UFUNCTION(BlueprintCallable, Category = "Graph Selection")
	void EndLaserSelect();
	void DrawLaserSelectRay();
	bool bLaserSelectActive = false;
	
	UFUNCTION(BlueprintCallable, Category = "Graph Database")
	void DeleteGraphNodeFromDatabase();
	UFUNCTION(BlueprintCallable, Category = "Graph Database")
	void DeleteGraphLinkFromDatabase();

	// --- High-level graph editing (mode-aware) ---
	// These route to local-only or database-then-local depending on Config.CreationMode.

	// Add a node: immediate in AutoGenerate/FromJson, HTTP-first in FromDatabase.
	UFUNCTION(BlueprintCallable, Category = "Graph Editing")
	void RequestAddGraphNode(FString NodeName, int32 LinkTargetNodeIndex = -1);

	// Remove the currently selected node: immediate in AutoGenerate/FromJson, HTTP-first in FromDatabase.
	UFUNCTION(BlueprintCallable, Category = "Graph Editing")
	void RequestRemoveSelectedGraphNode();

	// --- Database HTTP callbacks ---
	void OnDeleteGraphNodeHttpCompleted(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);

	// --- Incremental graph mutation (swap-remove, local only) ---
	// Removes a node by array index using swap-remove: moves the last node into the deleted slot,
	// fixes up all link indices, and removes any links that referenced the deleted node.
	// Returns true on success.
	bool RemoveGraphNodeByIndex(int32 NodeIndexToRemove);

	// Adds a node to the end of all parallel arrays, creates its visual, and returns the new index.
	// Pass LinkTargetNodeIndex >= 0 to also create a link to an existing node.
	int32 AddGraphNodeLocal(const FString& NodeName, const FString& NodeStringId, FVector NodeWorldPosition, int32 LinkTargetNodeIndex = -1);

	// --- Cleanup / reload ---
	void CleanUpObjects();
	void ReloadTheWholeGraph();
	
	// Temporary variables.
	bool bGraphRequesting = false;
	bool bGraphInitialized = false;
	
	bool bPredefinedPositionNeedsUpdate = true;
	bool bPrecheckSucceeded = true;

	// Server connection failure shutdown
	bool bServerConnectionFailed = false;
	float ServerConnectionShutdownTimer = 5.0f;
	FVector GraphOwnerPosition;
	bool bGPUResultValid = false;
	

	
	// Fires whenever the selected node changes (index is -1 when deselected).
	FOnGraphNodeSelectionChanged OnGraphNodeSelectionChanged;

	// Important Variables.
	int32 SelectedGraphNodeIndex = -1;
	FString SelectedGraphNodeName = "";
	
	// Total number of nodes in the graph (formerly: jnodessss)
	int32 TotalNodeCount;
	
	TMap<int32, FString> NodeIdToStringMap;
	TMap<FString, int32> StringToNodeIdMap;
	
	// Array of all graph nodes (formerly: all_nodes2)
	TArray<GraphNode> GraphNodes;
	
	// Array of all graph links/edges (formerly: all_links2)
	TArray<GraphLink> GraphLinks;
	
	TArray<FVector> predefined_positions;

	
	TArray<FVector> nodePositions;
	TArray<FVector> nodeVelocities;







	
	UPROPERTY(VisibleAnywhere, Instanced)
	TObjectPtr<UInstancedStaticMeshComponent> InstancedStaticMeshComponent;
	

	
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
	void UpdatePositionArray(bool log);
	void UpdateAlpha();
	void UpdateIterations();
	void GenerateTextRenderComponentAndAttach(FString name,int32 index);

	void Miscellaneous();
	void SetArrayLengths();
	void SetArrayValues();
	void InitializeArrays();
	bool GenerateObjectsForNodeAndLinkNew(); // Handles all creation modes via DataManager

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


	// New callback for DataManager
	UFUNCTION()
	void OnGraphDataLoadedCallback(bool bSuccess);

	// Physics simulator integration
	void InitializePhysicsSimulator();
	void CpuCalculateNew(); // CPU physics via PhysicsSimulator (with legacy fallback)

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

	// Register UObject pointers hidden inside GraphNodes/GraphLinks with the GC
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);







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
	void LogMessage(const FString& StringToLog, bool LOG=true, int SeverityLevel = 0, const FString& Prefix = TEXT("[Info]"))
	{
		if(Config.bEnableLogging)
		{
			LogMessageInternal(StringToLog, LOG, SeverityLevel, Prefix);
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



	
};


