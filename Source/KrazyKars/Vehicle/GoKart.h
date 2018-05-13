// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

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

UCLASS()
class KRAZYKARS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoKart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


private:
	void UpdateLocationFromVelocity(float DeltaTime);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveForward(float value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveRight(float value);

	void MoveForward(float value);
	void MoveRight(float value);

	void ApplyRotation(float DeltaTime);
	FVector CalculateCurrentForce() const;

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

	UPROPERTY(Replicated)
	FVector velocity;

	UPROPERTY(Replicated)
	float throttle;
	UPROPERTY(Replicated)
	float steeringThrow;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedTransformation)
	FTransform replicated_Transformation;

	UFUNCTION()
	void OnRep_ReplicatedTransformation();
};
