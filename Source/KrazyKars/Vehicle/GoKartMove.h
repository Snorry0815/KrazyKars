#pragma once

#include "CoreMinimal.h"
#include "GoKartMove.generated.h"

USTRUCT()
struct FGoKartMove
{
	GENERATED_BODY()

	UPROPERTY()
	float throttle;

	UPROPERTY()
	float steeringThrow;

	UPROPERTY()
	float deltaTime;

	UPROPERTY()
	float time;
};