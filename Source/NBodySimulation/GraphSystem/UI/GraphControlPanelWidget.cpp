// GraphControlPanelWidget.cpp

#include "GraphControlPanelWidget.h"
#include "KnowledgeGraph.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

void UGraphControlPanelWidget::InitializePanel(AKnowledgeGraph* InGraphActor)
{
	GraphActorRef = InGraphActor;
	if (!GraphActorRef) return;

	FGraphConfiguration& Cfg = GraphActorRef->Config;

	// Rendering toggles
	AddToggleButton(TEXT("Node Mesh"), [&Cfg]() -> bool& { return Cfg.bUseInstancedStaticMesh; });
	AddToggleButton(TEXT("Text Labels"), [&Cfg]() -> bool& { return Cfg.bUseTextRenderComponents; });
	AddToggleButton(TEXT("Link Mesh"), [&Cfg]() -> bool& { return Cfg.bUseLinkStaticMesh; });
	AddToggleButton(TEXT("Link Debug Lines"), [&Cfg]() -> bool& { return Cfg.bUseLinkDebugLine; });
	AddToggleButton(TEXT("Face Player"), [&Cfg]() -> bool& { return Cfg.bRotateTextToFacePlayer; });

	// Simulation toggles
	AddToggleButton(TEXT("Link Force"), [&Cfg]() -> bool& { return Cfg.bCalculateLinkForce; });
	AddToggleButton(TEXT("Many-Body Force"), [&Cfg]() -> bool& { return Cfg.bCalculateManyBodyForce; });
	AddToggleButton(TEXT("Parallel Processing"), [&Cfg]() -> bool& { return Cfg.bUseParallelProcessing; });

	// Debug
	AddToggleButton(TEXT("Logging"), [&Cfg]() -> bool& { return Cfg.bEnableLogging; });
	AddToggleButton(TEXT("Debug Grid"), [&Cfg]() -> bool& { return Cfg.bDrawDebugGrid; });

	// Action buttons
	AddActionButton(TEXT("Reload Graph"), [this]()
	{
		if (GraphActorRef)
		{
			GraphActorRef->ReloadTheWholeGraph();
		}
	});

	AddActionButton(TEXT("Remove Selected Node"), [this]()
	{
		if (GraphActorRef)
		{
			GraphActorRef->RemoveSelectedGraphNode();
		}
	});

	RefreshAllLabels();
}

void UGraphControlPanelWidget::TogglePanelVisibility()
{
	if (GetVisibility() == ESlateVisibility::Visible)
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		RefreshAllLabels();
		SetVisibility(ESlateVisibility::Visible);
	}
}

TSharedRef<SWidget> UGraphControlPanelWidget::RebuildWidget()
{
	PanelButtonContainer = SNew(SVerticalBox);

	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.85f))
		.Padding(FMargin(20.0f))
		[
			SNew(SVerticalBox)

			// Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 12)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Graph Controls")))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
			]

			// Scrollable button list
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					PanelButtonContainer.ToSharedRef()
				]
			]
		];
}

void UGraphControlPanelWidget::AddToggleButton(const FString& Label, TFunction<bool&()> ConfigGetter)
{
	TSharedPtr<STextBlock> LabelText;

	int32 ButtonIndex = ToggleButtons.Num();

	if (PanelButtonContainer.IsValid())
	{
		PanelButtonContainer->AddSlot()
			.AutoHeight()
			.Padding(0, 3)
			[
				SNew(SButton)
				.OnClicked_Lambda([this, ButtonIndex]() -> FReply
				{
					if (ButtonIndex < ToggleButtons.Num())
					{
						FToggleButtonInfo& Info = ToggleButtons[ButtonIndex];
						bool& ConfigValue = Info.ConfigGetter();
						ConfigValue = !ConfigValue;
						RefreshAllLabels();
					}
					return FReply::Handled();
				})
				.ContentPadding(FMargin(16.0f, 6.0f))
				[
					SAssignNew(LabelText, STextBlock)
					.Justification(ETextJustify::Left)
				]
			];
	}

	FToggleButtonInfo Info;
	Info.BaseLabel = Label;
	Info.ConfigGetter = ConfigGetter;
	Info.TextBlock = LabelText;
	ToggleButtons.Add(MoveTemp(Info));
}

void UGraphControlPanelWidget::AddActionButton(const FString& Label, TFunction<void()> ActionFunc)
{
	if (!PanelButtonContainer.IsValid()) return;

	PanelButtonContainer->AddSlot()
		.AutoHeight()
		.Padding(0, 3)
		[
			SNew(SButton)
			.OnClicked_Lambda([ActionFunc]() -> FReply
			{
				if (ActionFunc) ActionFunc();
				return FReply::Handled();
			})
			.ContentPadding(FMargin(16.0f, 6.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Label))
				.Justification(ETextJustify::Left)
			]
		];
}

void UGraphControlPanelWidget::RefreshAllLabels()
{
	for (FToggleButtonInfo& Info : ToggleButtons)
	{
		if (Info.TextBlock.IsValid() && Info.ConfigGetter)
		{
			bool bCurrentValue = Info.ConfigGetter();
			FString ToggleStateLabel = FString::Printf(TEXT("%s: %s"),
				*Info.BaseLabel,
				bCurrentValue ? TEXT("ON") : TEXT("OFF"));
			Info.TextBlock->SetText(FText::FromString(ToggleStateLabel));
		}
	}
}
