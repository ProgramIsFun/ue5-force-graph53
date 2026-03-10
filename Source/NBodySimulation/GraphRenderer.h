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
class Link77;
class Node77;

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
		TArray<Node77>& Nodes,
		const TArray<FVector>& InitialPositions
	);

	void UpdateNodePositions(
		const TArray<FVector>& NodePositions,
		const TArray<Node77>& Nodes
	);

	void RotateTextToFacePlayer(
		const TArray<FVector>& NodePositions,
		const TArray<Node77>& Nodes,
		const FVector& PlayerLocation
	);

	// Link rendering
	void InitializeLinkVisuals(
		TArray<Link77>& Links,
		UStaticMesh* LinkMesh,
		UMaterialInterface* LinkMaterial
	);

	void UpdateLinkPositions(
		const TArray<Link77>& Links,
		const TArray<FVector>& NodePositions,
		UWorld* World
	);

	// Text label management
	void SetTextSize(float Size, const TArray<Node77>& Nodes);
	void AdjustTextSize(float Delta, const TArray<Node77>& Nodes);

	// Cleanup
	void ClearAllVisuals(TArray<Node77>& Nodes, TArray<Link77>& Links);

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
	void CreateTextComponent(const FString& Text, int32 Index, TArray<Node77>& Nodes);
	void UpdateInstancedMeshTransforms(const TArray<FVector>& NodePositions);

	// Logging
	void LogMessage(const FString& Message, bool bForceLog = false) const;
};
