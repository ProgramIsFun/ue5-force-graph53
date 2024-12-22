// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameModeBase111.h"

#include "DefaultPawn2.h"

AMyGameModeBase111::AMyGameModeBase111(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Sprite")))
{
	// bNetLoadOnClient = false;
	// bPauseable = true;
	// bStartPlayersAsSpectators = false;

	DefaultPawnClass = ADefaultPawn2::StaticClass();
	// PlayerControllerClass = APlayerController::StaticClass();
	// PlayerStateClass = APlayerState::StaticClass();
	// GameStateClass = AGameStateBase::StaticClass();
	// HUDClass = AHUD::StaticClass();
	// GameSessionClass = AGameSession::StaticClass();
	// SpectatorClass = ASpectatorPawn::StaticClass();
	// ReplaySpectatorPlayerControllerClass = APlayerController::StaticClass();
	// ServerStatReplicatorClass = AServerStatReplicator::StaticClass();


}
