// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/Components/ReadyRideComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Level/Platform/RideRoot.h"
#include "Net/UnrealNetwork.h"
#include "Player/MAPlayerCharacter.h"

UReadyRideComponent::UReadyRideComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
}

void UReadyRideComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UReadyRideComponent, RidingRoot);
}

void UReadyRideComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AActor* OwnerActor = GetOwner())
	{
		PrevTickLocation = OwnerActor->GetActorLocation();
	}
	bPrevAttachedReady = IsAttachedReady();
	ApplyRideState(bPrevAttachedReady);
}

void UReadyRideComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	const bool bAttachedByReady = IsAttachedReady();

	const FVector CurrentLocation = OwnerActor->GetActorLocation();
	if (bAttachedByReady && DeltaTime > KINDA_SMALL_NUMBER)
	{
		FVector Delta = CurrentLocation - PrevTickLocation;
		Delta.Z = 0.f;
		AttachedMoveVelocity = Delta / DeltaTime;
		AttachedMoveSpeed = AttachedMoveVelocity.Size();
	}
	else
	{
		AttachedMoveVelocity = FVector::ZeroVector;
		AttachedMoveSpeed = 0.f;
	}

	PrevTickLocation = CurrentLocation;
}

void UReadyRideComponent::NotifyReadyRideAttachmentChanged(ARideRoot* InRideRoot)
{
	RidingRoot = InRideRoot;
	if (!RidingRoot)
	{
		AttachedMoveVelocity = FVector::ZeroVector;
		AttachedMoveSpeed = 0.f;
	}

	if (const AActor* OwnerActor = GetOwner())
	{
		PrevTickLocation = OwnerActor->GetActorLocation();
	}

	ApplyRideState(IsAttachedReady());
}

bool UReadyRideComponent::IsAttachedReady() const
{
	return RidingRoot != nullptr;
}

bool UReadyRideComponent::TryGetAttachedYaw(float& OutYaw) const
{
	if (!RidingRoot) return false;
	OutYaw = RidingRoot->GetActorRotation().Yaw;
	return true;
}

void UReadyRideComponent::RefreshRideCollisionMode()
{
	AMAPlayerCharacter* OwnerCharacter = Cast<AMAPlayerCharacter>(GetOwner());
	if (!OwnerCharacter) return;

	UCapsuleComponent* Capsule = OwnerCharacter->GetCapsuleComponent();
	if (!Capsule) return;

	const bool bNowRidingPlatform = IsAttachedReady();
	if (bNowRidingPlatform == bIsRidingPlatform) return;
	bIsRidingPlatform = bNowRidingPlatform;

	UWorld* World = GetWorld();
	if (!World) return;

	for (TActorIterator<AMAPlayerCharacter> It(World); It; ++It)
	{
		AMAPlayerCharacter* OtherPlayer = *It;
		if (!OtherPlayer || OtherPlayer == OwnerCharacter) continue;
		UpdateRideCollisionWithOtherPlayer(OwnerCharacter, OtherPlayer);
	}
}

void UReadyRideComponent::HandleOwnerBaseChanged()
{
	RefreshRideCollisionMode();
}

void UReadyRideComponent::OnRep_RidingRoot()
{
	const AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		PrevTickLocation = OwnerActor->GetActorLocation();
	}
	if (!RidingRoot)
	{
		AttachedMoveVelocity = FVector::ZeroVector;
		AttachedMoveSpeed = 0.f;
	}
	ApplyRideState(IsAttachedReady());
}

void UReadyRideComponent::ApplyRideState(bool bNowAttached)
{
	if (bNowAttached != bPrevAttachedReady)
	{
		HandleReplicatedAttachStateChanged(bNowAttached);
		bPrevAttachedReady = bNowAttached;
	}

	UpdateRideMovementMode(bNowAttached);
	UpdateTickPolicy(bNowAttached);
	RefreshRideCollisionMode();
}

void UReadyRideComponent::HandleReplicatedAttachStateChanged(bool bNowAttached)
{
	AMAPlayerCharacter* OwnerCharacter = Cast<AMAPlayerCharacter>(GetOwner());
	if (!OwnerCharacter) return;

	// Only reconcile simulated proxies. Server/local-owner flow remains untouched.
	if (OwnerCharacter->HasAuthority() || OwnerCharacter->IsLocallyControlled()) return;

	UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
	if (!MoveComp) return;

	MoveComp->StopMovementImmediately();
	MoveComp->ClearAccumulatedForces();
}

void UReadyRideComponent::UpdateRideMovementMode(bool bNowAttached) const
{
	const AMAPlayerCharacter* OwnerCharacter = Cast<AMAPlayerCharacter>(GetOwner());
	if (!OwnerCharacter) return;

	UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
	if (!MoveComp) return;

	(void)bNowAttached;

	if (MoveComp->MovementMode != MOVE_Walking)
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}
}

void UReadyRideComponent::UpdateTickPolicy(bool bAttachedByReady)
{
	if (bAttachedByReady)
	{
		PrimaryComponentTick.TickInterval = 0.f;
		SetComponentTickEnabled(true);
		return;
	}

	SetComponentTickEnabled(false);
}

void UReadyRideComponent::UpdateRideCollisionWithOtherPlayer(AMAPlayerCharacter* OwnerCharacter, AMAPlayerCharacter* OtherPlayer) const
{
	if (!OwnerCharacter || !OtherPlayer) return;

	UCapsuleComponent* Capsule = OwnerCharacter->GetCapsuleComponent();
	UCapsuleComponent* OtherCapsule = OtherPlayer->GetCapsuleComponent();
	if (!Capsule || !OtherCapsule) return;

	const UReadyRideComponent* OtherReadyRideComp = OtherPlayer->FindComponentByClass<UReadyRideComponent>();
	const bool bOtherRidingPlatform = OtherReadyRideComp ? OtherReadyRideComp->bIsRidingPlatform : false;
	const bool bShouldIgnorePair = bIsRidingPlatform || bOtherRidingPlatform;

	Capsule->IgnoreActorWhenMoving(OtherPlayer, bShouldIgnorePair);
	OtherCapsule->IgnoreActorWhenMoving(OwnerCharacter, bShouldIgnorePair);

	if (bShouldIgnorePair)
	{
		OwnerCharacter->MoveIgnoreActorAdd(OtherPlayer);
		OtherPlayer->MoveIgnoreActorAdd(OwnerCharacter);
	}
	else
	{
		OwnerCharacter->MoveIgnoreActorRemove(OtherPlayer);
		OtherPlayer->MoveIgnoreActorRemove(OwnerCharacter);
	}
}
