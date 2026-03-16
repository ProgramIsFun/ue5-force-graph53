// GraphControlPanelWidget.h
// Main control panel widget containing graph rendering control buttons

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GraphControlPanelWidget.generated.h"

class AKnowledgeGraph;

UCLASS()
class NBODYSIMULATION_API UGraphControlPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Initialize with a reference to the graph actor
	void InitializePanel(AKnowledgeGraph* InGraphActor);

	// Toggle visibility of the panel
	void TogglePanelVisibility();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	// Adds a toggle button that flips a bool config value
	void AddToggleButton(const FString& Label, TFunction<bool&()> ConfigGetter);

	// Adds an action button that runs a one-shot function
	void AddActionButton(const FString& Label, TFunction<void()> ActionFunc);

	void RefreshAllLabels();

	UPROPERTY()
	AKnowledgeGraph* GraphActorRef = nullptr;

	// Slate container
	TSharedPtr<SVerticalBox> PanelButtonContainer;

	// Track toggle buttons for label refresh
	struct FToggleButtonInfo
	{
		FString BaseLabel;
		TFunction<bool&()> ConfigGetter;
		TSharedPtr<STextBlock> TextBlock;
	};
	TArray<FToggleButtonInfo> ToggleButtons;
};
