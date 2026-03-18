// NodePropertyPanelWidget.cpp

#include "NodePropertyPanelWidget.h"
#include "KnowledgeGraph.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SOverlay.h"

void UNodePropertyPanelWidget::InitializePropertyPanel(AKnowledgeGraph* InGraphActor)
{
	GraphActorRef = InGraphActor;
}

void UNodePropertyPanelWidget::RefreshSelectedNodeProperties()
{
	// Guard: RebuildWidget may not have run yet
	if (!PanelRootBorder.IsValid() || !NodeTitleTextBlock.IsValid() || PropertyRows.Num() == 0)
	{
		return;
	}

	if (!GraphActorRef || GraphActorRef->SelectedGraphNodeIndex < 0)
	{
		PanelRootBorder->SetVisibility(EVisibility::Collapsed);
		return;
	}

	const int32 SelectedNodeIndex = GraphActorRef->SelectedGraphNodeIndex;
	const FString& SelectedNodeDisplayName = GraphActorRef->SelectedGraphNodeName;

	NodeTitleTextBlock->SetText(FText::FromString(SelectedNodeDisplayName));

	// Collect all properties to display
	TArray<TPair<FString, FString>> DisplayProperties;

	// Built-in: node index
	DisplayProperties.Emplace(TEXT("Index"), FString::FromInt(SelectedNodeIndex));

	// Built-in: string ID from the ID map (safe UPROPERTY TMap)
	if (const FString* NodeStringId = GraphActorRef->NodeIdToStringMap.Find(SelectedNodeIndex))
	{
		if (!NodeStringId->IsEmpty())
		{
			DisplayProperties.Emplace(TEXT("String Id"), *NodeStringId);
		}
	}

	// Properties from DataManager
	if (GraphActorRef->DataManager)
	{
		const TMap<FString, FString>* NodeProperties = GraphActorRef->DataManager->GetNodeProperties(SelectedNodeIndex);
		if (NodeProperties)
		{
			for (const auto& PropertyEntry : *NodeProperties)
			{
				DisplayProperties.Emplace(PropertyEntry.Key, PropertyEntry.Value);
			}
		}
	}

	// Fill pre-allocated rows; hide unused ones
	const int32 RowCount = FMath::Min(DisplayProperties.Num(), PropertyRows.Num());
	for (int32 RowIdx = 0; RowIdx < PropertyRows.Num(); ++RowIdx)
	{
		FPropertyRowWidgets& Row = PropertyRows[RowIdx];
		if (RowIdx < RowCount)
		{
			Row.KeyTextBlock->SetText(FText::FromString(DisplayProperties[RowIdx].Key));
			Row.ValueTextBlock->SetText(FText::FromString(DisplayProperties[RowIdx].Value));
			Row.RowBox->SetVisibility(EVisibility::Visible);
		}
		else
		{
			Row.RowBox->SetVisibility(EVisibility::Collapsed);
		}
	}

	PanelRootBorder->SetVisibility(EVisibility::Visible);
}

TSharedRef<SWidget> UNodePropertyPanelWidget::RebuildWidget()
{
	TSharedPtr<SVerticalBox> PropertyColumnBox;

	// The inner panel with the dark background and content
	SAssignNew(PanelRootBorder, SBorder)
		.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.85f))
		.Padding(FMargin(14.0f))
		.Visibility(EVisibility::Collapsed)
		[
			SNew(SBox)
			.MinDesiredWidth(220.0f)
			.MaxDesiredWidth(400.0f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 8)
				[
					SAssignNew(NodeTitleTextBlock, STextBlock)
					.Text(FText::FromString(TEXT("No Selection")))
					.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.9f, 0.3f)))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.MaxHeight(300.0f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						SAssignNew(PropertyColumnBox, SVerticalBox)
					]
				]
			]
		];

	// Pre-allocate a fixed number of property rows (hidden by default)
	PropertyRows.Empty();
	PropertyRows.SetNum(MaxPropertyRows);

	for (int32 RowIdx = 0; RowIdx < MaxPropertyRows; ++RowIdx)
	{
		FPropertyRowWidgets& Row = PropertyRows[RowIdx];

		PropertyColumnBox->AddSlot()
			.AutoHeight()
			.Padding(0, 2)
			[
				SAssignNew(Row.RowBox, SHorizontalBox)
				.Visibility(EVisibility::Collapsed)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(Row.KeyTextBlock, STextBlock)
					.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8, 0, 0, 0)
				[
					SAssignNew(Row.ValueTextBlock, STextBlock)
					.ColorAndOpacity(FSlateColor(FLinearColor::White))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				]
			];
	}

	// Wrap in a full-screen overlay that pins the panel to the upper-right corner
	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(FMargin(0.0f, 20.0f, 20.0f, 0.0f))
		[
			PanelRootBorder.ToSharedRef()
		];
}
