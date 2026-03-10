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
};
