// GraphRenderer.h
// Handles all graph visualization: nodes, links, text labels

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GraphConfiguration.h"
#include "GraphRenderer.generated.h"

// Forward declarations
class GraphLink;
class GraphNode;

/**
 * Manages all visual representation of the graph
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NBODYSIMULATION_API UGraphRenderer : public UActorComponent
{
	GENERATED_BODY()

public:
	UGraphRenderer();

	// Initialize renderer with configuration
	void Initialize(const FGraphConfiguration& Config, AActor* OwnerActor);

	// Node rendering
	void InitializeNodeVisuals(
		int32 NodeCount,
		TArray<GraphNode>& Nodes,
		const TArray<FVector>& InitialPositions
	);

	void UpdateNodePositions(
		const TArray<FVector>& NodePositions,
		const TArray<GraphNode>& Nodes
	);

	void RotateTextToFacePlayer(
		const TArray<FVector>& NodePositions,
		const TArray<GraphNode>& Nodes,
		const FVector& PlayerLocation
	);

	// Link rendering
	void InitializeLinkVisuals(
		TArray<GraphLink>& Links,
		UStaticMesh* LinkMesh,
		UMaterialInterface* LinkMaterial
	);

	void UpdateLinkPositions(
		const TArray<GraphLink>& Links,
		const TArray<FVector>& NodePositions,
		UWorld* World
	);

	// Text label management
	void SetTextSize(float Size, const TArray<GraphNode>& Nodes);
	void AdjustTextSize(float Delta, const TArray<GraphNode>& Nodes);

	// Cleanup
	void ClearAllVisuals(TArray<GraphNode>& Nodes, TArray<GraphLink>& Links);

	// Get instanced mesh component (for external access)
	UInstancedStaticMeshComponent* GetInstancedMeshComponent() const { return InstancedMeshComponent; }

protected:
	virtual void BeginPlay() override;

private:
	// Configuration
	FGraphConfiguration Config;
	AActor* OwnerActor = nullptr;

	// Node rendering
	UPROPERTY()
	UInstancedStaticMeshComponent* InstancedMeshComponent = nullptr;

	TArray<FTransform> NodeTransforms;

	// Helper methods
	void CreateTextComponent(const FString& Text, int32 Index, TArray<GraphNode>& Nodes);
	void UpdateInstancedMeshTransforms(const TArray<FVector>& NodePositions);

	// Logging
	void LogMessage(const FString& Message, bool bForceLog = false) const;
};
