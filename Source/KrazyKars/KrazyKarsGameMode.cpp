// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "KrazyKarsGameMode.h"
#include "KrazyKarsPawn.h"
#include "KrazyKarsHud.h"

AKrazyKarsGameMode::AKrazyKarsGameMode()
{
	DefaultPawnClass = AKrazyKarsPawn::StaticClass();
	HUDClass = AKrazyKarsHud::StaticClass();
}
