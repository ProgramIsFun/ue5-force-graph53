// GraphPlayerController.h
// Player controller that handles ESC to toggle the graph control panel

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GraphPlayerController.generated.h"

class UGraphControlPanelWidget;
class UNodePropertyPanelWidget;
class AKnowledgeGraph;

UCLASS()
class NBODYSIMULATION_API AGraphPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGraphPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void ToggleGraphControlPanel();
	void FindAndBindGraphActor();
	void OnLaserSelectPressed();
	void OnLaserSelectReleased();
	void EnsureGraphActorCached();
	void OnSelectedGraphNodeChanged(int32 SelectedNodeIndex);

	UPROPERTY()
	UGraphControlPanelWidget* GraphControlPanel = nullptr;

	UPROPERTY()
	UNodePropertyPanelWidget* NodePropertyPanel = nullptr;

	UPROPERTY()
	AKnowledgeGraph* CachedGraphActor = nullptr;

	bool bGraphPanelVisible = false;
};
