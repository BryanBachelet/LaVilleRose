// Copyright Epic Games, Inc. All Rights Reserved.

#include "EuropeRPGGameMode.h"
#include "EuropeRPGPlayerController.h"
#include "EuropeRPGCharacter.h"
#include "UObject/ConstructorHelpers.h"

AEuropeRPGGameMode::AEuropeRPGGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AEuropeRPGPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}