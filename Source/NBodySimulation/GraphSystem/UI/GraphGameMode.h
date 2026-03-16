// GraphGameMode.h
// Game mode that wires in the GraphPlayerController

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GraphGameMode.generated.h"

UCLASS()
class NBODYSIMULATION_API AGraphGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGraphGameMode();
};
