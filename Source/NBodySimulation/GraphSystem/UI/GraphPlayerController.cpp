// GraphPlayerController.cpp

#include "GraphPlayerController.h"
#include "GraphControlPanelWidget.h"
#include "KnowledgeGraph.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

AGraphPlayerController::AGraphPlayerController()
{
}

void AGraphPlayerController::BeginPlay()
{
	Super::BeginPlay();
	FindAndBindGraphActor();
}

void AGraphPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("ToggleGraphPanel", IE_Pressed, this, &AGraphPlayerController::ToggleGraphControlPanel);

	// Home key to toggle graph control panel
	InputComponent->BindKey(EKeys::Home, IE_Pressed, this, &AGraphPlayerController::ToggleGraphControlPanel);

	// PgUp key to select node by look-at (crosshair pick)
	InputComponent->BindKey(EKeys::PageUp, IE_Pressed, this, &AGraphPlayerController::SelectNodeByLookAt);
}

void AGraphPlayerController::ToggleGraphControlPanel()
{
	if (!GraphControlPanel)
	{
		// Panel not created yet, try to find graph actor and create it
		FindAndBindGraphActor();
		if (!GraphControlPanel) return;
	}

	GraphControlPanel->TogglePanelVisibility();
	bGraphPanelVisible = (GraphControlPanel->GetVisibility() == ESlateVisibility::Visible);

	if (bGraphPanelVisible)
	{
		SetShowMouseCursor(true);
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
	else
	{
		SetShowMouseCursor(false);
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
	}
}

void AGraphPlayerController::FindAndBindGraphActor()
{
	if (GraphControlPanel) return;

	// Find the KnowledgeGraph actor in the world
	AActor* FoundGraphActor = nullptr;
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AKnowledgeGraph::StaticClass(), FoundActors);

	if (FoundActors.Num() > 0)
	{
		FoundGraphActor = FoundActors[0];
	}

	AKnowledgeGraph* GraphActor = Cast<AKnowledgeGraph>(FoundGraphActor);
	if (!GraphActor) return;

	// Cache the graph actor for direct key bindings (e.g. PgUp select)
	CachedGraphActor = GraphActor;

	// Create the panel widget
	GraphControlPanel = CreateWidget<UGraphControlPanelWidget>(this);
	if (GraphControlPanel)
	{
		GraphControlPanel->AddToViewport(10);
		GraphControlPanel->SetVisibility(ESlateVisibility::Collapsed);
		GraphControlPanel->InitializePanel(GraphActor);
	}
}

void AGraphPlayerController::SelectNodeByLookAt()
{
	if (!CachedGraphActor)
	{
		// Try to find it if not cached yet
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AKnowledgeGraph::StaticClass(), FoundActors);
		if (FoundActors.Num() > 0)
		{
			CachedGraphActor = Cast<AKnowledgeGraph>(FoundActors[0]);
		}
	}

	if (CachedGraphActor)
	{
		CachedGraphActor->SelectGraphNodeByLookAt();
	}
}
