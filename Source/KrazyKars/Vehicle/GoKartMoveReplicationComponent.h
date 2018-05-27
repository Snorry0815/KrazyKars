// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartState.h"
#include "GoKartMoveReplicationComponent.generated.h"

struct FHermiteCubicSpline
{
public:
	FHermiteCubicSpline(float Client_TimeSinceUpdate, float Client_TimeBetweenLastUpdates,
	    const FTransform& Client_StartTransformation, const FTransform& serverStateTransform
		, const FVector& Client_StartVelocity, const FVector& serverStateVelocity);

	FVector CreateCurrentLocation() const;
	FVector CreateCurrentVelocity() const;
	FQuat CreateCurrentRotation() const;

private:
	const float alpha;
	const FVector startLocation;
	const FVector targetLocation;
	const FQuat startRotation;
	const FQuat targetRotation;
	const float velocityDerivative;
	const FVector startDerivative;
	const FVector targetDerivative;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARS_API UGoKartMoveReplicationComponent : public UActorComponent
{
GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoKartMoveReplicationComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	void ClearAcknowledgedMoves(const FGoKartMove& lastServerMove);
	void UpdateServerState(const FGoKartMove& move);

	void Client_Tick(float DeltaTime);

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FGoKartState rep_ServerState;

	UFUNCTION()
	void OnRep_ServerState();
	void SimulatedProxy_OnRep_ServerState();
	void AutonomousProxy_OnRep_ServerState();


	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove move);

	TArray<FGoKartMove> unackknowledgedMoves;
	float Client_TimeSinceUpdate;
	float Client_TimeBetweenLastUpdates;
	FTransform Client_StartTransformation;
	FVector Client_StartVelocity;

	class UGoKartMovementComponent* goKartMovement;

	UPROPERTY()
	USceneComponent* meshOffsetRoot;

	UFUNCTION(Blueprintcallable)
	void SetMeshOffsetRoot(USceneComponent* root) { meshOffsetRoot = root; }
};
