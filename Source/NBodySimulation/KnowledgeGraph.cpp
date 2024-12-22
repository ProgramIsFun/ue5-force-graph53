// Fill out your copyright notice in the Description page of Project Settings.


#include "KnowledgeGraph.h"

#include <random>

// #define ENABLE_LOGGING 1
#include "utillllllssss.h"
#include "Kismet/GameplayStatics.h"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::White,text)

AKnowledgeGraph::~AKnowledgeGraph()
{
	ll("AKnowledgeGraph::~AKnowledgeGraph", true, 2);
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

}

void AKnowledgeGraph::BeginDestroy()
{
	ll("AKnowledgeGraph::BeginDestroy", true, 2);
	
	FNBodySimModule::Get().EndRendering();
	

	Super::BeginDestroy();
}

void AKnowledgeGraph::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ll("AKnowledgeGraph::EndPlay", true, 2);
	ll("EndPlayReason: " + FString::FromInt((int)EndPlayReason), true, 2);
	// if (use_shaders)
	// {
	// 	FNBodySimModule::Get().EndRendering();
	// }
	Super::EndPlay(EndPlayReason);
}


void AKnowledgeGraph::BeginPlay()
{
	Super::BeginPlay();

	ClearLogFile();
}


void AKnowledgeGraph::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!precheck_succeed)
	{
		ll("Prechecks failed. ", true, 2);
		return;
	}

	if (!graph_initialized)
	{
		if (graph_requested)
		{
			ll("Graph is requested but not initialized. ", true, 2);
		}else
		{
			graph_requested = true;
			prepare();
		}
		return;
	}
	

	
	if(iterationsf<10)
	{
		ll("The reason of this section is because the first few frames seems "
	 "to be of sink between the gpu and the cpu. ", false, 2); 
		iterationsf+=1;
		return;
	}
	
	main_function(DeltaTime);

}
