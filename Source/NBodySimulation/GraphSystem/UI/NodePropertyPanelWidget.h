// NodePropertyPanelWidget.h
// Displays key-value properties of the currently selected graph node in the upper-right corner.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NodePropertyPanelWidget.generated.h"

class AKnowledgeGraph;

UCLASS()
class NBODYSIMULATION_API UNodePropertyPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializePropertyPanel(AKnowledgeGraph* InGraphActor);

	// Call this whenever the selected node changes to refresh displayed properties.
	void RefreshSelectedNodeProperties();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	static constexpr int32 MaxPropertyRows = 16;

	UPROPERTY()
	AKnowledgeGraph* GraphActorRef = nullptr;

	// Slate containers
	TSharedPtr<STextBlock> NodeTitleTextBlock;
	TSharedPtr<SBorder> PanelRootBorder;

	// Pre-allocated property row text blocks (key + value per row)
	struct FPropertyRowWidgets
	{
		TSharedPtr<STextBlock> KeyTextBlock;
		TSharedPtr<STextBlock> ValueTextBlock;
		TSharedPtr<SHorizontalBox> RowBox;
	};
	TArray<FPropertyRowWidgets> PropertyRows;
};
