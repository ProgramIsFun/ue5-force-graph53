// GraphRenderer.cpp
// Implementation of graph visualization

#include "GraphRenderer.h"
#include "KnowledgeGraph.h" // For Node77, Link77
#include "DrawDebugHelpers.h"

UGraphRenderer::UGraphRenderer()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGraphRenderer::BeginPlay()
{
	Super::BeginPlay();
}

void UGraphRenderer::Initialize(const FGraphConfiguration& InConfig, AActor* InOwnerActor)
{
	Config = InConfig;
	OwnerActor = InOwnerActor;

	// Create instanced mesh component if needed
	if (Config.bUseInstancedStaticMesh && !InstancedMeshComponent)
	{
		InstancedMeshComponent = NewObject<UInstancedStaticMeshComponent>(
			OwnerActor,
			UInstancedStaticMeshComponent::StaticClass(),
			TEXT("NodeInstancedMesh")
		);

		if (InstancedMeshComponent)
		{
			InstancedMeshComponent->SetupAttachment(OwnerActor->GetRootComponent());
			InstancedMeshComponent->RegisterComponent();
			LogMessage("Created InstancedStaticMeshComponent");
		}
	}

	LogMessage("GraphRenderer initialized");
}

void UGraphRenderer::InitializeNodeVisuals(
	int32 NodeCount,
	TArray<Node77>& Nodes,
	const TArray<FVector>& InitialPositions)
{
	if (Config.bUseInstancedStaticMesh && InstancedMeshComponent && Config.NodeMesh)
	{
		// Prepare transforms for instanced rendering
		NodeTransforms.SetNum(NodeCount);
		float Scale = Config.InstancedMeshSize;

		for (int32 i = 0; i < NodeCount; i++)
		{
			FTransform Transform(
				FRotator::ZeroRotator,
				InitialPositions[i],
				FVector(Scale, Scale, Scale)
			);
			NodeTransforms[i] = Transform;
		}

		// Set the mesh
		InstancedMeshComponent->SetStaticMesh(Config.NodeMesh);

		// Add all instances at once
		InstancedMeshComponent->AddInstances(NodeTransforms, false);
		
		LogMessage("Initialized " + FString::FromInt(NodeCount) + " node instances");
	}
}


void UGraphRenderer::UpdateNodePositions(
	const TArray<FVector>& NodePositions,
	const TArray<Node77>& Nodes)
{
	// Update instanced mesh positions
	if (Config.bUseInstancedStaticMesh && InstancedMeshComponent)
	{
		for (int32 i = 0; i < NodePositions.Num(); i++)
		{
			NodeTransforms[i].SetTranslation(NodePositions[i]);
		}

		InstancedMeshComponent->BatchUpdateInstancesTransforms(
			0,
			NodeTransforms,
			false,
			true
		);
	}

	// Update text component positions
	if (Config.bUseTextRenderComponents)
	{
		for (int32 i = 0; i < NodePositions.Num() && i < Nodes.Num(); i++)
		{
			if (Nodes[i].textComponent)
			{
				Nodes[i].textComponent->SetWorldLocation(NodePositions[i]);
			}
		}
	}
}

void UGraphRenderer::RotateTextToFacePlayer(
	const TArray<FVector>& NodePositions,
	const TArray<Node77>& Nodes,
	const FVector& PlayerLocation)
{
	if (!Config.bUseTextRenderComponents || !Config.bRotateTextToFacePlayer)
	{
		return;
	}

	for (int32 i = 0; i < NodePositions.Num() && i < Nodes.Num(); i++)
	{
		if (Nodes[i].textComponent)
		{
			FVector ToPlayer = PlayerLocation - NodePositions[i];
			ToPlayer.Normalize();
			FRotator NewRotation = FRotationMatrix::MakeFromX(ToPlayer).Rotator();
			Nodes[i].textComponent->SetWorldRotation(NewRotation);
		}
	}
}

void UGraphRenderer::InitializeLinkVisuals(
	TArray<Link77>& Links,
	UStaticMesh* LinkMesh,
	UMaterialInterface* LinkMaterial)
{
	if (!Config.bUseLinkStaticMesh || !LinkMesh || !OwnerActor)
	{
		return;
	}

	for (int32 i = 0; i < Links.Num(); i++)
	{
		UStaticMeshComponent* CylinderMesh = NewObject<UStaticMeshComponent>(
			OwnerActor,
			FName(*FString::Printf(TEXT("LinkMesh%d"), i))
		);

		if (CylinderMesh)
		{
			CylinderMesh->AttachToComponent(
				OwnerActor->GetRootComponent(),
				FAttachmentTransformRules::KeepRelativeTransform
			);
			CylinderMesh->RegisterComponent();
			CylinderMesh->SetWorldScale3D(FVector(1, 1, 1));
			CylinderMesh->SetStaticMesh(LinkMesh);

			if (LinkMaterial)
			{
				CylinderMesh->SetMaterial(0, LinkMaterial);
			}

			Links[i].edgeMesh = CylinderMesh;
		}
	}

	LogMessage("Initialized " + FString::FromInt(Links.Num()) + " link meshes");
}

void UGraphRenderer::UpdateLinkPositions(
	const TArray<Link77>& Links,
	const TArray<FVector>& NodePositions,
	UWorld* World)
{
	for (const Link77& Link : Links)
	{
		const FVector Location1 = NodePositions[Link.source];
		const FVector Location2 = NodePositions[Link.target];

		// Update static mesh links
		if (Config.bUseLinkStaticMesh && Link.edgeMesh)
		{
			FVector ForwardVector = Location2 - Location1;
			float CylinderHeight = ForwardVector.Size();
			FRotator Rotation = FRotationMatrix::MakeFromZ(ForwardVector).Rotator();

			Link.edgeMesh->SetWorldLocation(Location1);
			Link.edgeMesh->SetWorldScale3D(FVector(
				Config.LinkThickness,
				Config.LinkThickness,
				Config.LinkLengthFineTune * CylinderHeight
			));
			Link.edgeMesh->SetWorldRotation(Rotation);
		}

		// Draw debug lines
		if (Config.bUseLinkDebugLine && World)
		{
			DrawDebugLine(
				World,
				Location1,
				Location2,
				FColor::Red,
				false,
				-1,
				0,
				10.0f
			);
		}
	}
}

void UGraphRenderer::SetTextSize(float Size, const TArray<Node77>& Nodes)
{
	if (!Config.bUseTextRenderComponents)
	{
		return;
	}

	for (const Node77& Node : Nodes)
	{
		if (Node.textComponent)
		{
			Node.textComponent->SetWorldSize(Size);
		}
	}
}

void UGraphRenderer::AdjustTextSize(float Delta, const TArray<Node77>& Nodes)
{
	if (!Config.bUseTextRenderComponents)
	{
		return;
	}

	for (const Node77& Node : Nodes)
	{
		if (Node.textComponent)
		{
			float CurrentSize = Node.textComponent->WorldSize;
			Node.textComponent->SetWorldSize(CurrentSize + Delta);
		}
	}
}

void UGraphRenderer::ClearAllVisuals(TArray<Node77>& Nodes, TArray<Link77>& Links)
{
	// Clear text components
	if (Config.bUseTextRenderComponents)
	{
		for (Node77& Node : Nodes)
		{
			if (Node.textComponent && Node.textComponent->IsRegistered())
			{
				Node.textComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
				Node.textComponent->UnregisterComponent();
				Node.textComponent->DestroyComponent();
				Node.textComponent = nullptr;
			}
		}
	}

	// Clear instanced meshes
	if (Config.bUseInstancedStaticMesh && InstancedMeshComponent)
	{
		InstancedMeshComponent->ClearInstances();
	}

	// Clear link meshes
	for (Link77& Link : Links)
	{
		if (Link.edgeMesh && Link.edgeMesh->IsRegistered())
		{
			Link.edgeMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			Link.edgeMesh->UnregisterComponent();
			Link.edgeMesh->DestroyComponent();
			Link.edgeMesh = nullptr;
		}
	}

	LogMessage("Cleared all visuals");
}

void UGraphRenderer::CreateTextComponent(const FString& Text, int32 Index, TArray<Node77>& Nodes)
{
	if (!OwnerActor || !Config.bUseTextRenderComponents)
	{
		return;
	}

	UTextRenderComponent* TextComponent = NewObject<UTextRenderComponent>(
		OwnerActor,
		FName("TextComponent" + Text)
	);

	if (TextComponent)
	{
		TextComponent->SetText(FText::FromString(Text));
		TextComponent->SetupAttachment(OwnerActor->GetRootComponent());
		TextComponent->SetWorldSize(Config.TextSize);
		TextComponent->RegisterComponent();
		
		if (Nodes.IsValidIndex(Index))
		{
			Nodes[Index].textComponent = TextComponent;
		}
	}
}

void UGraphRenderer::LogMessage(const FString& Message, bool bForceLog) const
{
	if (Config.bEnableLogging || bForceLog)
	{
		UE_LOG(LogTemp, Log, TEXT("[GraphRenderer] %s"), *Message);
	}
}
