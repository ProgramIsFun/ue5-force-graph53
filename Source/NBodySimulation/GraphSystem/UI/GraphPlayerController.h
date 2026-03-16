// GraphPlayerController.h
// Player controller that handles ESC to toggle the graph control panel

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GraphPlayerController.generated.h"

class UGraphControlPanelWidget;

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

	UPROPERTY()
	UGraphControlPanelWidget* GraphControlPanel = nullptr;

	bool bGraphPanelVisible = false;
};
