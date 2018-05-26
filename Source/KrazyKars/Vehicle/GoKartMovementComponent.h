// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.generated.h"

struct FGoKartMove;


UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARS_API UGoKartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoKartMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetThrottle(float value) { throttle = value; }
	void SetSteeringThrow(float value) { steeringThrow = value; }

	const FVector& GetVelocity() const { return velocity; }
	void SetVelocity(const FVector& value) { velocity = value; }

	void SimulateMove(FGoKartMove move);
	FGoKartMove CreateMove(float DeltaTime) const;

private:
	void UpdateLocationFromVelocity(float DeltaTime);


	void ApplyRotation(const FGoKartMove& move);
	FVector CalculateMoveForce(const FGoKartMove& move) const;

	UPROPERTY(EditAnywhere)
	float mass = 1000.0f;

	UPROPERTY(EditAnywhere)
	float maxDrivingForce = 10000.0f;

	UPROPERTY(EditAnywhere)
	float minimumTurningRadius = 10.0f;

	UPROPERTY(EditAnywhere)
	float dragCoefficient = 16.0f;

	UPROPERTY(EditAnywhere)
	float rollingResistanceCoefficient = 0.015f;

	FVector velocity;

	float throttle;
	float steeringThrow;
};
