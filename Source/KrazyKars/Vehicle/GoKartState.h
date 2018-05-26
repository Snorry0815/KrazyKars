#pragma once

#include "CoreMinimal.h"
#include "Transform.h"
#include "GoKartMove.h"
#include "GoKartState.generated.h"

USTRUCT()
struct FGoKartState
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform transform;

	UPROPERTY()
	FVector velocity;


	UPROPERTY()
	FGoKartMove lastMove;
};

