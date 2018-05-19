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

	if (IsLocallyControlled())
	{
		FGoKartMove move;
		move.deltaTime = DeltaTime;
		move.throttle = throttle;
		move.steeringThrow = steeringThrow;
		// TODO: move.time
		Server_SendMove(move); 
		SimulateMove(move);
	}

	FVector location = GetActorLocation();
	auto text = FString::Printf(TEXT("%s Position(%f, %f, %f)"), *GetEnumText(Role), location.X, location.Y, location.Z);
	DrawDebugString(GetWorld(), FVector(0, 0, 100), text, this, FColor::White, DeltaTime);
}

void AGoKart::SimulateMove(FGoKartMove move)
{
	FVector force = CalculateMoveForce(move);
	FVector acceleration = force / mass;
	velocity += acceleration * move.deltaTime;

	ApplyRotation(move);

	UpdateLocationFromVelocity(move.deltaTime);
}

FVector AGoKart::CalculateMoveForce(const FGoKartMove& move) const
{
	FVector force = GetActorForwardVector() * maxDrivingForce * move.throttle;
	force -= velocity.GetSafeNormal() * velocity.SizeSquared() * dragCoefficient;

	auto* world = GetWorld();
	float g = -world->GetGravityZ() / 100;
	force -= velocity.GetSafeNormal() * g * mass * rollingResistanceCoefficient;

	return force;
}

void AGoKart::ApplyRotation(const FGoKartMove& move)
{
	float dx = FVector::DotProduct(GetActorForwardVector(), velocity) * move.deltaTime;
	float rotationAngle = dx * move.steeringThrow / minimumTurningRadius;

	FQuat rotationDelta(GetActorUpVector(), rotationAngle);
	AddActorWorldRotation(rotationDelta);
	velocity = rotationDelta.RotateVector(velocity);
}

void AGoKart::OnRep_ServerState()
{
	SetActorTransform(rep_ServerState.transform);
	velocity = rep_ServerState.velocity;
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
}

void AGoKart::MoveRight(float value)
{
	steeringThrow = value;
}

void AGoKart::Server_SendMove_Implementation(FGoKartMove move)
{
	SimulateMove(move);
	rep_ServerState.lastMove = move;
	rep_ServerState.transform = GetActorTransform();
	rep_ServerState.velocity = velocity;
}

bool AGoKart::Server_SendMove_Validate(FGoKartMove move)
{
	// TODO: do this
	return true;
}

void AGoKart::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGoKart, rep_ServerState);
}
