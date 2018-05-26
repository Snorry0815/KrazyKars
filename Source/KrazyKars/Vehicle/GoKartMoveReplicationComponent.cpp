#include "GoKartMoveReplicationComponent.h"
#include "UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
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

// Sets default values for this component's properties
UGoKartMoveReplicationComponent::UGoKartMoveReplicationComponent()
	: Client_TimeSinceUpdate(0.0f)
	, Client_TimeBetweenLastUpdates(0.0f)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
}


// Called when the game starts
void UGoKartMoveReplicationComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	goKartMovement = GetOwner()->FindComponentByClass<UGoKartMovementComponent>();
}


// Called every frame
void UGoKartMoveReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ENetRole Role = GetOwnerRole();
	const FGoKartMove& lastMove = goKartMovement->GetLastMove();
	if (Role == ROLE_AutonomousProxy)
	{
		unackknowledgedMoves.Add(lastMove);
		Server_SendMove(lastMove);
	}

	// if we are server and in control of the pawn
	if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		UpdateServerState(lastMove);
	}

	if (Role == ROLE_SimulatedProxy)
	{
		Client_Tick(DeltaTime);
	}

	FVector location = GetOwner()->GetActorLocation();
	auto text = FString::Printf(TEXT("%s Position(%f, %f, %f)"), *GetEnumText(Role), location.X, location.Y, location.Z);
	DrawDebugString(GetWorld(), FVector(0, 0, 100), text, GetOwner(), FColor::White, DeltaTime);
}

void UGoKartMoveReplicationComponent::Client_Tick(float DeltaTime)
{
	Client_TimeSinceUpdate += DeltaTime;
	if (Client_TimeBetweenLastUpdates < KINDA_SMALL_NUMBER)
		return;

	float alpha = FMath::Clamp(Client_TimeSinceUpdate / Client_TimeBetweenLastUpdates, 0.0f, 1.0f);
	GetOwner()->SetActorLocation(FMath::LerpStable(Client_StartTransformation.GetLocation(), rep_ServerState.transform.GetLocation(), alpha));
	GetOwner()->SetActorRotation(FQuat::Slerp(Client_StartTransformation.GetRotation(), rep_ServerState.transform.GetRotation(), alpha));
}

void UGoKartMoveReplicationComponent::OnRep_ServerState()
{
	switch (GetOwnerRole()) { 
	case ROLE_None:
		break;
	case ROLE_SimulatedProxy:
		SimulatedProxy_OnRep_ServerState();
		break;
	case ROLE_AutonomousProxy:
		AutonomousProxy_OnRep_ServerState();
		break;
	case ROLE_Authority:
		break;
	case ROLE_MAX: 
		break;

	default: ;
	}
}

void UGoKartMoveReplicationComponent::SimulatedProxy_OnRep_ServerState()
{
	Client_TimeBetweenLastUpdates = Client_TimeSinceUpdate;
	Client_TimeSinceUpdate = 0.0f;
	Client_StartTransformation = GetOwner()->GetActorTransform();
}

void UGoKartMoveReplicationComponent::AutonomousProxy_OnRep_ServerState()
{
	if (goKartMovement == nullptr)
		return;

	GetOwner()->SetActorTransform(rep_ServerState.transform);

	goKartMovement->SetVelocity(rep_ServerState.velocity);
	ClearAcknowledgedMoves(rep_ServerState.lastMove);
	for (const auto& move : unackknowledgedMoves)
	{
		goKartMovement->SimulateMove(move);
	}
}

void UGoKartMoveReplicationComponent::ClearAcknowledgedMoves(const FGoKartMove& lastServerMove)
{
	int32 index = unackknowledgedMoves.IndexOfByPredicate([&lastServerMove](const FGoKartMove& current) { return current.time == lastServerMove.time; });
	if (index != INDEX_NONE)
	{
		unackknowledgedMoves.RemoveAt(0, index + 1);
	}
}

void UGoKartMoveReplicationComponent::UpdateServerState(const FGoKartMove& move)
{
	rep_ServerState.lastMove = move;
	rep_ServerState.transform = GetOwner()->GetActorTransform();
	rep_ServerState.velocity = goKartMovement->GetVelocity();
}

void UGoKartMoveReplicationComponent::Server_SendMove_Implementation(FGoKartMove move)
{
	goKartMovement->SimulateMove(move);
	UpdateServerState(move);
}

bool UGoKartMoveReplicationComponent::Server_SendMove_Validate(FGoKartMove move)
{
	// TODO: do this
	return true;
}

void UGoKartMoveReplicationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGoKartMoveReplicationComponent, rep_ServerState);
}

