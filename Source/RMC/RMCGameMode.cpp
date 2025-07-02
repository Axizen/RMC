// Copyright Epic Games, Inc. All Rights Reserved.

#include "RMCGameMode.h"
#include "RMCCharacter.h"
#include "UObject/ConstructorHelpers.h"

ARMCGameMode::ARMCGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
