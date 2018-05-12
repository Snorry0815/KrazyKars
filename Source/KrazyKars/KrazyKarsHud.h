// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/HUD.h"
#include "KrazyKarsHud.generated.h"


UCLASS(config = Game)
class AKrazyKarsHud : public AHUD
{
	GENERATED_BODY()

public:
	AKrazyKarsHud();

	/** Font used to render the vehicle info */
	UPROPERTY()
	UFont* HUDFont;

	// Begin AHUD interface
	virtual void DrawHUD() override;
	// End AHUD interface
};
