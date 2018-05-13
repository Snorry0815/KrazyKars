// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "UnrealNetwork.h"

namespace
{

FString GetEnumText(ENetRole role)
{
	switch (role) { 
	case ROLE_None: 
		return "None";
	case ROLE_SimulatedProxy: 
		return "SimulatedProxy";
	case ROLE_AutonomousProxy: 
		return "AutonomousProxy";
	case ROLE_Authority:
		return "Authority";
	default: 
		return "Error"; 
	}
}

}

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
}


// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector force = CalculateCurrentForce();
	FVector acceleration = force / mass;
	velocity += acceleration * DeltaTime;

	ApplyRotation(DeltaTime);

	UpdateLocationFromVelocity(DeltaTime);

	if (HasAuthority())
	{
		replicated_Transformation = GetActorTransform();
	}

	FVector location = GetActorLocation();
	auto text = FString::Printf(TEXT("%s Position(%f, %f, %f)"), *GetEnumText(Role), location.X, location.Y, location.Z);
	DrawDebugString(GetWorld(), FVector(0, 0, 100), text, this, FColor::White, DeltaTime);
}

FVector AGoKart::CalculateCurrentForce() const
{
	FVector force = GetActorForwardVector() * maxDrivingForce * throttle;
	force -= velocity.GetSafeNormal() * velocity.SizeSquared() * dragCoefficient;

	auto* world = GetWorld();
	float g = -world->GetGravityZ() / 100;
	force -= velocity.GetSafeNormal() * g * mass * rollingResistanceCoefficient;

	return force;
}

void AGoKart::OnRep_ReplicatedTransformation()
{
	SetActorTransform(replicated_Transformation);
}

void AGoKart::ApplyRotation(float DeltaTime)
{
	float dx = FVector::DotProduct(GetActorForwardVector(), velocity) * DeltaTime;
	float rotationAngle = dx * steeringThrow / minimumTurningRadius;

	FQuat rotationDelta(GetActorUpVector(), rotationAngle);
	AddActorWorldRotation(rotationDelta);
	velocity = rotationDelta.RotateVector(velocity);
}

void AGoKart::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector move = velocity * DeltaTime * 100.0f;

	FHitResult result;
	AddActorWorldOffset(move, true, &result);
	if (result.IsValidBlockingHit())
		velocity = FVector::ZeroVector;
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}

void AGoKart::MoveForward(float value)
{
	throttle = value;
	Server_MoveForward(value);
}

void AGoKart::MoveRight(float value)
{
	steeringThrow = value;
	Server_MoveRight(value);
}

void AGoKart::Server_MoveForward_Implementation(float value)
{
	throttle = value;
}

bool AGoKart::Server_MoveForward_Validate(float value)
{
	return value <= 1.0f && value >= -1.0f;
}

void AGoKart::Server_MoveRight_Implementation(float value)
{
	steeringThrow = value;
}

bool AGoKart::Server_MoveRight_Validate(float value)
{
	return value <= 1.0f && value >= -1.0f;
}

void AGoKart::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGoKart, replicated_Transformation);
	DOREPLIFETIME(AGoKart, velocity);
	DOREPLIFETIME(AGoKart, throttle);
	DOREPLIFETIME(AGoKart, steeringThrow);
}
