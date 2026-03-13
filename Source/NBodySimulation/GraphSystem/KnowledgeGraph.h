// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "KnowledgeNode.h"
#include "KnowledgeEdge.h"
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
	AKnowledgeEdge* edge;
	UStaticMeshComponent* edgeMesh;
	GraphLink(int32 SourceNodeIndex, int32 TargetNodeIndex, AKnowledgeEdge* edge)
	{
		this->SourceNodeIndex = SourceNodeIndex;
		this->TargetNodeIndex = TargetNodeIndex;
		this->edge = edge;
	}
	GraphLink(int32 SourceNodeIndex, int32 TargetNodeIndex)
	{
		this->SourceNodeIndex = SourceNodeIndex;
		this->TargetNodeIndex = TargetNodeIndex;
	}
	GraphLink()
	{
	}
};

class GraphNode
{
public:
	int id;
	AKnowledgeNode* node;
	UTextRenderComponent* textComponent;
	GraphNode(int id, AKnowledgeNode* node)
	{
		this->id = id;
		this->node = node;
	}
	GraphNode(int id, UTextRenderComponent* textComponent)
	{
		this->id = id;
		this->textComponent = textComponent;
	}
};

UCLASS()
class NBODYSIMULATION_API AKnowledgeGraph : public AActor
{
	GENERATED_BODY()
	
public:

	// Function exposed to blueprints
	UFUNCTION(BlueprintCallable, Category = "YourCategory111111111111222222222")
	void set_text_size_of_all_nodes1112(float size);
	UFUNCTION(BlueprintCallable, Category = "YourCategory111111111111222222222")
	void increase_or_decrease_text_size_of_all_nodes1112(bool increase, float size);
	UFUNCTION(BlueprintCallable, Category = "YourCategory111111111111222222222")
	void increase_text_size_of_all_nodes1112(float size);
	UFUNCTION(BlueprintCallable, Category = "YourCategory111111111111222222222")
	void decrease_text_size_of_all_nodes1112(float size);

	UFUNCTION(BlueprintCallable, Category = "YourCategory111111111111222222222")
	void update_position_of_all_nodes_to_database1113();
	UFUNCTION(BlueprintCallable, Category = "YourCategory111111111111222222222")
	void add_link_to_database1114();
	UFUNCTION(BlueprintCallable, Category = "YourCategory111111111111222222222")
	void add_node_to_database1115(FString NodeName);
	void add_node_to_database1115httpCompleted(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);
	void clean_up_objects();
	void reload_the_whole_graph();
	void late_add_node(FString NodeName, FString id, FVector location);

	UFUNCTION(BlueprintCallable, Category = "YourCategory111111111111222222222")
	void select_closest_node_from_player222();
	
	UFUNCTION(BlueprintCallable, Category = "YourCategory111111111111222222222")
	void delete_node_from_database1116();
	UFUNCTION(BlueprintCallable, Category = "YourCategory111111111111222222222")
	void delete_link_from_database1117();
	
	// Temporary variables.
	bool graph_requesting = false;
	bool graph_initialized = false;

	bool refresh_whole_graph_again_after_editing = true;
	
	bool use_predefined_position_should_update_once = true;
	bool precheck_succeed = true;
	FVector current_own_position;
	bool GPUvalid = false;
	

	
	// Important Variables.
	TSharedPtr<FJsonObject> JsonObject1;
	TArray<TMap<FString, FString>> AllNodeProperties;

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
	TArray<UTextRenderComponent*> TextComponents11111111111111111111;
	

	
	TMap<int32, FString> fileIndexToPath = {
		{0, "statered.json"},
		{1, "state - 2024-06-18T223257.374.json"},
		{2, "state777777777.json"},
	};



	// Internal functions. 
	void debug_test();
	void gpu_get_positions();
	bool main_function(float DeltaTime);
	void rotate_to_face_player111();
	void pass_parameters_to_shader_management();
	void post_generate_graph();
	void prepare();
	void update_parameter_in_shader(float DeltaTime);
	bool is_graph_stabilized(bool log);
	void cpu_calculate();
	void update_position_array(bool log);
	void update_alpha();
	void print_out_location_of_the_node();
	void update_iterations();
	bool generate_actor_and_register(AKnowledgeNode*& kn);
	void generate_text_render_component_and_attach(FString name,int32 index);
	void get_number_of_nodes();
	void create_one_to_one_mapping();
	void miscellaneous();
	void set_array_lengths();
	void set_array_values();
	void initialize_arrays();
	bool generate_objects_for_node_and_link();
	bool generate_objects_for_node_and_link_new(); // New version using DataManager
	void extracting_property_list_and_store();
	void deal_with_predefined_location();
	void default_generate_graph_method();
	void add_edge(int32 id, int32 source, int32 target);
	void initialize_node_position();
	void initialize_node_position_individual(int index);
	FVector GetPlayerLocation();
	FVector GetLocationInFrontOfPlayer();
	void calculate_bias_and_strength_of_links();
	bool generate_actor_for_a_link(GraphLink& link);
	void apply_force();
	void calculate_link_force_and_update_velocity();
	void calculate_charge_force_and_update_velocity();
	void calculate_centre_force_and_update_position();
	void update_node_world_position_according_to_position_array();
	void update_position_array_according_to_velocity_array();
	void update_link_position();
	void request_graph_httpCompleted(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);
	void request_graph_http();
	void request_a_graph();
	void debug_error_request(FHttpRequestPtr Request, FHttpResponsePtr Response);

	// New callback for DataManager
	UFUNCTION()
	void OnGraphDataLoadedCallback(bool bSuccess);

	// Physics simulator integration
	void InitializePhysicsSimulator();
	void cpu_calculate_new(); // New version using PhysicsSimulator

	// Renderer integration
	void update_node_world_position_according_to_position_array_new();
	void update_link_position_new();
	void rotate_to_face_player_new();



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
	int iterationsf = 0;
	float iterations = 0;





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


