// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "KnowledgeNode.h"
#include "KnowledgeEdge.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "CoreMinimal.h"
#include "octreeeeeeeeee3.h"
#include "utillllllssss.h"

#include "NBodySimModule.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"


///////////////////////////////////////////////////////////////////////////////////////

#include "KnowledgeGraph.generated.h"

class Link77
{
public:
	int32 source;
	int32 target;
	float bias;
	float strength;
	float distance;
	AKnowledgeEdge* edge;
	UStaticMeshComponent* edgeMesh;
	Link77(int32 source, int32 target, AKnowledgeEdge* edge)
	{
		this->source = source;
		this->target = target;
		this->edge = edge;
	}
	Link77(int32 source, int32 target)
	{
		this->source = source;
		this->target = target;
	}
	Link77()
	{
	}
};

class Node77
{
public:
	int id;
	AKnowledgeNode* node;
	UTextRenderComponent* textComponent;
	Node77(int id, AKnowledgeNode* node)
	{
		this->id = id;
		this->node = node;
	}
	Node77(int id, UTextRenderComponent* textComponent)
	{
		this->id = id;
		this->textComponent = textComponent;
	}
};

UENUM(BlueprintType)
enum class CGM : uint8
{
	GENERATE UMETA(DisplayName = "autoGenerate11111"),
	JSON UMETA(DisplayName = "json22222"),
	DATABASE UMETA(DisplayName = "database333333")
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

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
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
	
	int32 jnodessss;
	
	TMap<int32, FString> id_to_string;
	TMap<FString, int32> string_to_id;
	TArray<Node77> all_nodes2;
	TArray<Link77> all_links2;
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
	void extracting_property_list_and_store();
	void deal_with_predefined_location();
	void default_generate_graph_method();
	void add_edge(int32 id, int32 source, int32 target);
	void initialize_node_position();
	void initialize_node_position_individual(int index);
	FVector get_player_location727();
	FVector get_location_of_somewhere_in_front_of_player727();
	void calculate_bias_and_strength_of_links();
	bool generate_actor_for_a_link(Link77& link);
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



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	float universal_graph_scale = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	bool use_shaders = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	int32 use_shaders_debug = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	CGM cgm= CGM::GENERATE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	bool use_predefined_location = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	bool use_predefined_locationand_then_center_to_current_actor = true;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	bool initialize_using_actor_location = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	int32 use_json_file_index = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	bool cpu_linkc = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	// Calculate many body force or not. 
	bool cpu_manybody = true;

	// Turn on logging or not. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	bool use_logging = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	bool node_use_instance_static_mesh = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	float node_use_instance_static_mesh_size = 0.3f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	UStaticMesh* SelectedMesh1111111111111;
	// 3////////////////////
	// Use TextRenderComponent or not
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	bool node_use_text_render_components = true;
	// The size of TextRenderComponent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	float text_size = 10;

	// rotation to face the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	bool rotate_to_face_player = true;


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Link setting////////////////////////////////////////////////////////////////////////////////////////////////////


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	bool link_use_static_mesh = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	UStaticMesh* link_use_static_mesh_mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	UMaterialInterface* CylinderMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	float link_use_static_mesh_thickness = 0.05f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	float link_use_static_mesh_length_fine_tune = 0.01f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	bool link_use_debug_line = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	bool update_link_before_stabilize = true;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	int jnodes1 = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	bool cpu_use_parallel = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	bool cpu_many_body_use_brute_force = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	bool connect_to_previous = true;




	
	// Internal hard coded variables.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
	float alpha = 1;
	int iterationsf = 0;
	float iterations = 0;
	float alphaMin = 0.001;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attributes)
	float alphaDecay = 1 - FMath::Pow(alphaMin, 1.0 / 300);
	float edgeDistance = 30;
	float nodeStrength = -60;
	float distancemin = 1;
	float distancemax = 10000000;
	float alphaTarget = 0;
	float velocityDecay = 0.6;
	float initialAngle = PI * (3 - sqrt(5));
	float initialRadius = 10;
	float node_use_actor_size = 0.3f;




	// Unreal engine actor functions. 
	AKnowledgeGraph();
	virtual ~AKnowledgeGraph() override;
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;







	// Helper functions.
	template <typename Func, typename... Args>
	auto timeThisMemberFunction(const char* functionName, Func function, Args&&... args)
	{
		// auto start = std::chrono::high_resolution_clock::now();
		double StartTime = FPlatformTime::Seconds();
		(this->*function)(std::forward<Args>(args)...);
		double EndTime = FPlatformTime::Seconds();
		double ElapsedTime = EndTime - StartTime;
		lll("Elapsed time For " + FString(functionName) + ": " + FString::SanitizeFloat(ElapsedTime) + " seconds");
		return ElapsedTime;
	}
	void qq()
	{
		// This doesn't block the game thread. It just sent a request to end the game. 
		UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit,
		                               false);
	}
	void s()
	{
		FNBodySimModule::Get().EndRendering();
	}
	void ll(const FString& StringToLog, bool LOG=false, int SeverityLevel = 0, const FString& Prefix = TEXT("[Info]"))
	{
		if(use_logging)
		{
			ll2(StringToLog, LOG, SeverityLevel, Prefix);
		}
	}
	void lp(const FString& StringToLog, bool LOG=false, int SeverityLevel = 0, const FString& Prefix = TEXT("[Info]"))
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


