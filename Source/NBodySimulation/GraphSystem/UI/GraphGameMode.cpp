// GraphGameMode.cpp

#include "GraphGameMode.h"
#include "GraphPlayerController.h"

AGraphGameMode::AGraphGameMode()
{
	PlayerControllerClass = AGraphPlayerController::StaticClass();
}
