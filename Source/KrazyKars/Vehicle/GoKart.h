// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKartState.h"
#include "GoKart.generated.h"

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
	void MoveForward(float value);
	void MoveRight(float value);

	void ClearAcknowledgedMoves(const FGoKartMove& lastServerMove);


	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FGoKartState rep_ServerState;

	UFUNCTION()
	void OnRep_ServerState();


	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove move);

	TArray<FGoKartMove> unackknowledgedMoves;

	UPROPERTY(EditAnywhere)
	class UGoKartMovementComponent* goKartMovement;
};
