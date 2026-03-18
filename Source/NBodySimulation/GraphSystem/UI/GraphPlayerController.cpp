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

	// PgUp key: hold to aim laser, release to select node
	InputComponent->BindKey(EKeys::PageUp, IE_Pressed, this, &AGraphPlayerController::OnLaserSelectPressed);
	InputComponent->BindKey(EKeys::PageUp, IE_Released, this, &AGraphPlayerController::OnLaserSelectReleased);
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

void AGraphPlayerController::OnLaserSelectPressed()
{
	EnsureGraphActorCached();
	if (CachedGraphActor)
	{
		CachedGraphActor->BeginLaserSelect();
	}
}

void AGraphPlayerController::OnLaserSelectReleased()
{
	if (CachedGraphActor)
	{
		CachedGraphActor->EndLaserSelect();
	}
}

void AGraphPlayerController::EnsureGraphActorCached()
{
	if (CachedGraphActor) return;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AKnowledgeGraph::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		CachedGraphActor = Cast<AKnowledgeGraph>(FoundActors[0]);
	}
}
