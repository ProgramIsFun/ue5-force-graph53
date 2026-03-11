// Configuration struct for KnowledgeGraph
// Separates configuration from logic for better maintainability

#pragma once

#include "CoreMinimal.h"
#include "GraphConfiguration.generated.h"

UENUM(BlueprintType)
enum class EGraphCreationMode : uint8
{
	AutoGenerate UMETA(DisplayName = "Auto Generate"),
	FromJson UMETA(DisplayName = "From JSON File"),
	FromDatabase UMETA(DisplayName = "From Database")
};

USTRUCT(BlueprintType)
struct FGraphConfiguration
{
	GENERATED_BODY()

	// General Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
	float UniversalGraphScale = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
	EGraphCreationMode CreationMode = EGraphCreationMode::AutoGenerate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
	int32 AutoGenerateNodeCount = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
	int32 JsonFileIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
	bool bConnectToAdjacentNodeOnly = true;

	// Position Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
	bool bUsePredefinedLocation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
	bool bCenterPredefinedLocationToActor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
	bool bInitializeUsingActorLocation = true;

	// Simulation Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
	bool bUseGPUShaders = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
	int32 ShaderDebugLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
	bool bCalculateLinkForce = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
	bool bCalculateManyBodyForce = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
	bool bUseParallelProcessing = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
	bool bUseBruteForceForManyBody = true;

	// Node Rendering Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node Rendering")
	bool bUseInstancedStaticMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node Rendering")
	float InstancedMeshSize = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node Rendering")
	UStaticMesh* NodeMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node Rendering")
	bool bUseTextRenderComponents = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node Rendering")
	float TextSize = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node Rendering")
	bool bRotateTextToFacePlayer = true;

	// Link Rendering Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Link Rendering")
	bool bUseLinkStaticMesh = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Link Rendering")
	UStaticMesh* LinkMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Link Rendering")
	UMaterialInterface* LinkMaterial = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Link Rendering")
	float LinkThickness = 0.05f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Link Rendering")
	float LinkLengthFineTune = 0.01f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Link Rendering")
	bool bUseLinkDebugLine = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Link Rendering")
	bool bUpdateLinkBeforeStabilize = true;

	// Debug Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bEnableLogging = false;

	// Physics Simulation Parameters
	// Reference: d3-force (https://github.com/d3/d3-force)
	// These values are based on d3-force's velocity Verlet integration implementation
	// DO NOT MODIFY unless you understand the physics implications
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float Alpha = 1.0f; // Simulation "temperature" - decreases over time to stabilize the graph
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float AlphaMin = 0.001f; // Minimum alpha threshold - simulation stops when alpha reaches this value
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float AlphaTarget = 0.0f; // Target alpha value (0 = fully stabilized)
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float AlphaDecay = 0.0228f; // Alpha decay rate - controls cooling schedule (300 iterations to reach alphaMin from 1.0)
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float EdgeDistance = 30.0f; // Default link distance - from d3-force default
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float NodeStrength = -60.0f; // Charge force strength (negative = repulsion) - from d3-force default
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float DistanceMin = 1.0f; // Minimum distance for force calculations to prevent singularities
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float DistanceMax = 10000000.0f; // Maximum distance for force calculations
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float VelocityDecay = 0.6f; // Velocity damping factor (0.6 = 40% velocity retained per tick) - from d3-force
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float InitialAngle = 2.39996322972865332f; // Golden angle for initial node positioning (137.5 degrees in radians) - PI * (3 - sqrt(5))
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float InitialRadius = 10.0f; // Initial radius for spiral node placement - from d3-force
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float NodeActorSize = 0.3f; // Visual size multiplier for node actors
};
