// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "UnrealNetwork.h"
#include "GoKartMovementComponent.h"

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

	goKartMovement = CreateDefaultSubobject<UGoKartMovementComponent>(TEXT("GoKartMovementComponent"));
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

	if (Role == ROLE_AutonomousProxy)
	{
		FGoKartMove move = goKartMovement->CreateMove(DeltaTime);

		unackknowledgedMoves.Add(move);
		goKartMovement->SimulateMove(move);
		Server_SendMove(move);
	}

	// if we are server and in control of the pawn
	if (Role == ROLE_Authority && GetRemoteRole() == ROLE_SimulatedProxy)
	{
		FGoKartMove move = goKartMovement->CreateMove(DeltaTime);
		Server_SendMove(move);
	}

	if (Role == ROLE_SimulatedProxy)
	{
		FGoKartMove move = rep_ServerState.lastMove;
		move.deltaTime = DeltaTime;
		goKartMovement->SimulateMove(move);
	}

	FVector location = GetActorLocation();
	auto text = FString::Printf(TEXT("%s Position(%f, %f, %f)"), *GetEnumText(Role), location.X, location.Y, location.Z);
	DrawDebugString(GetWorld(), FVector(0, 0, 100), text, this, FColor::White, DeltaTime);
}

void AGoKart::OnRep_ServerState()
{
	SetActorTransform(rep_ServerState.transform);
	goKartMovement->SetVelocity(rep_ServerState.velocity);
	ClearAcknowledgedMoves(rep_ServerState.lastMove);
	for (const auto& move : unackknowledgedMoves)
	{
		goKartMovement->SimulateMove(move);
	}
}

void AGoKart::ClearAcknowledgedMoves(const FGoKartMove& lastServerMove)
{
	int32 index = unackknowledgedMoves.IndexOfByPredicate([&lastServerMove](const FGoKartMove& current) { return current.time == lastServerMove.time; });
	if (index != INDEX_NONE)
	{
		unackknowledgedMoves.RemoveAt(0, index + 1);
	}
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
	goKartMovement->SetThrottle(value);
}

void AGoKart::MoveRight(float value)
{
	goKartMovement->SetSteeringThrow(value);
}

void AGoKart::Server_SendMove_Implementation(FGoKartMove move)
{
	goKartMovement->SimulateMove(move);
	rep_ServerState.lastMove = move;
	rep_ServerState.transform = GetActorTransform();
	rep_ServerState.velocity = goKartMovement->GetVelocity();
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
