// Fill out your copyright notice in the Description page of Project Settings.
#include "KnowledgeGraph.h"
#include "NBodyUtils.h"
AKnowledgeGraph::~AKnowledgeGraph()
{
	// Do NOT call UObject methods here — the object may already be partially GC'd.
	// Use BeginDestroy() or EndPlay() for cleanup logging.
}
AKnowledgeGraph::AKnowledgeGraph()
	: Super()
{
	FNBodySimModule::Get().EndRendering();
	PrimaryActorTick.bCanEverTick = true;
	InstancedStaticMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(
					TEXT("InstancedStaticMeshComponent"));
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickGroup = TG_DuringPhysics;

	// Note: DataManager and PhysicsSimulator are created in BeginPlay
	// because they are UObjects, not components
}

void AKnowledgeGraph::BeginDestroy()
{
	LogMessage("AKnowledgeGraph::BeginDestroy", true, 2);
	FNBodySimModule::Get().EndRendering();

	// Clear arrays containing non-trivial types (FString in GraphNode)
	// before the C++ destructor runs, since GC may have already
	// invalidated the backing memory by that point.
	GraphNodes.Empty();

	Super::BeginDestroy();
}

void AKnowledgeGraph::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	LogMessage("AKnowledgeGraph::EndPlay", true, 2);
	LogMessage("EndPlayReason: " + FString::FromInt((int)EndPlayReason), true, 2);
	Super::EndPlay(EndPlayReason);
}

void AKnowledgeGraph::BeginPlay()
{
	Super::BeginPlay();
	ClearLogFile();
	
	// Create the data manager and physics simulator
	// (UObjects must be created at runtime, not in constructor)
	DataManager = NewObject<UGraphDataManager>(this);
	PhysicsSimulator = NewObject<UGraphPhysicsSimulator>(this);
	
	// Create and initialize the renderer (UActorComponent)
	Renderer = NewObject<UGraphRenderer>(this, UGraphRenderer::StaticClass(), TEXT("GraphRenderer"));
	if (Renderer)
	{
		Renderer->RegisterComponent();
		Renderer->Initialize(Config, this);
	}
	
	// Bind to data manager's delegate
	if (DataManager)
	{
		DataManager->DatabaseQueryUrl = Config.GraphDatabaseQueryUrl;
		DataManager->NodeCreateUrl = Config.GraphNodeCreateUrl;
		DataManager->NodePositionSyncUrl = Config.GraphNodePositionSyncUrl;
		DataManager->OnGraphDataLoaded.AddDynamic(this, &AKnowledgeGraph::OnGraphDataLoadedCallback);
	}
}

void AKnowledgeGraph::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LogMessage("tick is called. ", true, 2);

	// Handle server connection failure shutdown with countdown
	if (bServerConnectionFailed)
	{
		ServerConnectionShutdownTimer -= DeltaTime;
		int32 SecondsRemaining = FMath::CeilToInt(FMath::Max(ServerConnectionShutdownTimer, 0.0f));
		GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Red,
			FString::Printf(TEXT("Server connection failed. Shutting down in %d seconds..."), SecondsRemaining));
		if (ServerConnectionShutdownTimer <= 0.0f)
		{
			LogMessage("Server connection failure shutdown timer expired. Quitting game.", true, 2);
			QuitGame();
		}
		return;
	}

	if (!bPrecheckSucceeded)
	{
		LogMessage("Prechecks failed! requested to end game", true, 2);
		QuitGame();
		
	}else
	{
		if (!bGraphInitialized)
		{
			if (bGraphRequesting)
			{
				LogMessage("Graph is requested but not initialized. ", true, 2);
			}else
			{
				LogMessage("Graph is not initialized and also not requested. We need to request a graph. ", true, 2);
				bGraphRequesting = true;
				Prepare();
			}
			
		}else
		{
			LogMessage("Graph is initialized. We have enough data to move on.  ", true, 2);
			if(FrameSkipCounter<10)
			{
				LogMessage("The reason of this section is because the first few frames seems "
			 "to be of sink between the gpu and the cpu. ", true, 2); 
				FrameSkipCounter+=1;
				return;
			}
	
			MainFunction(DeltaTime);
		}
	}
	
	if (bLaserSelectActive)
	{
		DrawLaserSelectRay();
	}

	if (Config.bDrawDebugGrid)
	{
		DrawDebugGrid(GetActorLocation());
	}
}

void AKnowledgeGraph::DrawDebugGrid(const FVector& Center, int32 GridSize, float CellSize)
{
	// Draw horizontal grids (XY plane) at multiple Z levels
	for (int32 z = -GridSize; z <= GridSize; z++)
	{
		for (int32 i = -GridSize; i <= GridSize; i++)
		{
			// Lines parallel to X-axis (running along X direction) - GREEN
			DrawDebugLine(
				GetWorld(),
				FVector(Center.X - GridSize * CellSize, Center.Y + i * CellSize, Center.Z + z * CellSize),
				FVector(Center.X + GridSize * CellSize, Center.Y + i * CellSize, Center.Z + z * CellSize),
				FColor::Green,
				false, -1.0f, 0, 2.0f
			);
			
			// Lines parallel to Y-axis (running along Y direction) - RED
			DrawDebugLine(
				GetWorld(),
				FVector(Center.X + i * CellSize, Center.Y - GridSize * CellSize, Center.Z + z * CellSize),
				FVector(Center.X + i * CellSize, Center.Y + GridSize * CellSize, Center.Z + z * CellSize),
				FColor::Red,
				false, -1.0f, 0, 2.0f
			);
		}
	}
	
	// Draw vertical lines (along Z-axis) at each grid intersection - BLUE
	for (int32 x = -GridSize; x <= GridSize; x++)
	{
		for (int32 y = -GridSize; y <= GridSize; y++)
		{
			DrawDebugLine(
				GetWorld(),
				FVector(Center.X + x * CellSize, Center.Y + y * CellSize, Center.Z - GridSize * CellSize),
				FVector(Center.X + x * CellSize, Center.Y + y * CellSize, Center.Z + GridSize * CellSize),
				FColor::Blue,
				false, -1.0f, 0, 2.0f
			);
		}
	}
	
	// Draw coordinate axes (larger for visibility)
	DrawDebugLine(GetWorld(), Center, Center + FVector(1000, 0, 0), FColor::Red, false, -1.0f, 0, 5.0f);    // X-axis (Red)
	DrawDebugLine(GetWorld(), Center, Center + FVector(0, 1000, 0), FColor::Green, false, -1.0f, 0, 5.0f);  // Y-axis (Green)
	DrawDebugLine(GetWorld(), Center, Center + FVector(0, 0, 1000), FColor::Blue, false, -1.0f, 0, 5.0f);   // Z-axis (Blue)
}
