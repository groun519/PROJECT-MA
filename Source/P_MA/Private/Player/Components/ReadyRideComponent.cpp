// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/Components/ReadyRideComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Level/Platform/RideRoot.h"
#include "Player/MAPlayerCharacter.h"

UReadyRideComponent::UReadyRideComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UReadyRideComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AActor* OwnerActor = GetOwner())
	{
		PrevTickLocation = OwnerActor->GetActorLocation();
	}
	bPrevAttachedReady = IsAttachedReady();
	UpdateTickPolicy(bPrevAttachedReady);

	RefreshRideCollisionMode();
}

void UReadyRideComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	const bool bAttachedByReady = IsAttachedReady();
	if (bAttachedByReady != bPrevAttachedReady)
	{
		HandleReplicatedAttachStateChanged(bAttachedByReady);
		bPrevAttachedReady = bAttachedByReady;
		UpdateTickPolicy(bAttachedByReady);
	}

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

void UReadyRideComponent::NotifyReadyRideAttachmentChanged(bool bInAttachedReady)
{
	if (!bInAttachedReady)
	{
		AttachedMoveVelocity = FVector::ZeroVector;
		AttachedMoveSpeed = 0.f;
	}

	if (const AActor* OwnerActor = GetOwner())
	{
		PrevTickLocation = OwnerActor->GetActorLocation();
	}

	UpdateTickPolicy(bInAttachedReady);

	RefreshRideCollisionMode();
}

bool UReadyRideComponent::IsAttachedReady() const
{
	return IsAttachedToRideRoot();
}

bool UReadyRideComponent::TryGetAttachedYaw(float& OutYaw) const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return false;

	const USceneComponent* RootComp = OwnerActor->GetRootComponent();
	if (!RootComp) return false;

	const USceneComponent* AttachParent = RootComp->GetAttachParent();
	if (!AttachParent) return false;

	const AActor* AttachParentOwner = AttachParent->GetOwner();
	if (!AttachParentOwner || !AttachParentOwner->IsA(ARideRoot::StaticClass())) return false;

	OutYaw = AttachParent->GetComponentRotation().Yaw;
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

bool UReadyRideComponent::IsAttachedToRideRoot() const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return false;

	const USceneComponent* RootComp = OwnerActor->GetRootComponent();
	if (!RootComp) return false;

	const USceneComponent* AttachParent = RootComp->GetAttachParent();
	if (!AttachParent) return false;

	const AActor* AttachParentOwner = AttachParent->GetOwner();
	return AttachParentOwner && AttachParentOwner->IsA(ARideRoot::StaticClass());
}

void UReadyRideComponent::HandleReplicatedAttachStateChanged(bool bNowAttached) const
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

void UReadyRideComponent::UpdateTickPolicy(bool bAttachedByReady)
{
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	const bool bClientPawn = PawnOwner && !PawnOwner->HasAuthority();

	if (bAttachedByReady)
	{
		PrimaryComponentTick.TickInterval = 0.f;
		SetComponentTickEnabled(true);
		return;
	}

	if (bClientPawn)
	{
		// Client pawns (owner + simulated) poll attachment at low frequency
		// so replicated attach edges can be detected and tick can switch back to per-frame.
		PrimaryComponentTick.TickInterval = 0.1f;
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
