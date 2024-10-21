// Copyright Epic Games, Inc. All Rights Reserved.

#include "VRLerpTestGameMode.h"
#include "VRLerpTestCharacter.h"
#include "UObject/ConstructorHelpers.h"

AVRLerpTestGameMode::AVRLerpTestGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
