// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKartMovementComponent.h"
#include "GoKart.h"
#include "Engine/World.h"


// Sets default values for this component's properties
UGoKartMovementComponent::UGoKartMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGoKartMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGoKartMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if ((GetOwnerRole() == ROLE_AutonomousProxy) || (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy))
	{
		lastMove = CreateMove(DeltaTime);
		SimulateMove(lastMove);
	}
}



FGoKartMove UGoKartMovementComponent::CreateMove(float DeltaTime) const
{
	FGoKartMove move;
	move.time = GetWorld()->TimeSeconds;
	move.deltaTime = DeltaTime;
	move.throttle = throttle;
	move.steeringThrow = steeringThrow;
	return move;
}


void UGoKartMovementComponent::SimulateMove(FGoKartMove move)
{
	FVector force = CalculateMoveForce(move);
	FVector acceleration = force / mass;
	velocity += acceleration * move.deltaTime;

	ApplyRotation(move);

	UpdateLocationFromVelocity(move.deltaTime);
}

FVector UGoKartMovementComponent::CalculateMoveForce(const FGoKartMove& move) const
{
	FVector force = GetOwner()->GetActorForwardVector() * maxDrivingForce * move.throttle;
	force -= velocity.GetSafeNormal() * velocity.SizeSquared() * dragCoefficient;

	auto* world = GetWorld();
	float g = -world->GetGravityZ() / 100;
	force -= velocity.GetSafeNormal() * g * mass * rollingResistanceCoefficient;

	return force;
}

void UGoKartMovementComponent::ApplyRotation(const FGoKartMove& move)
{
	float dx = FVector::DotProduct(GetOwner()->GetActorForwardVector(), velocity) * move.deltaTime;
	float rotationAngle = dx * move.steeringThrow / minimumTurningRadius;

	FQuat rotationDelta(GetOwner()->GetActorUpVector(), rotationAngle);
	GetOwner()->AddActorWorldRotation(rotationDelta);
	velocity = rotationDelta.RotateVector(velocity);
}

void UGoKartMovementComponent::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector move = velocity * DeltaTime * 100.0f;

	FHitResult result;
	GetOwner()->AddActorWorldOffset(move, true, &result);
	if (result.IsValidBlockingHit())
		velocity = FVector::ZeroVector;
}
