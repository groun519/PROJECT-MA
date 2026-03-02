// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/Components/ReadyRideComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "Level/Platform/PlatformRoot.h"
#include "Player/MAPlayerCharacter.h"

UReadyRideComponent::UReadyRideComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UReadyRideComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AActor* OwnerActor = GetOwner())
	{
		PrevTickLocation = OwnerActor->GetActorLocation();
	}

	RefreshRideCollisionMode();
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

	RefreshRideCollisionMode();
}

bool UReadyRideComponent::IsAttachedReady() const
{
	return IsAttachedToPlatformRoot();
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
	if (!AttachParentOwner || !AttachParentOwner->IsA(APlatformRoot::StaticClass())) return false;

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

bool UReadyRideComponent::IsAttachedToPlatformRoot() const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return false;

	const USceneComponent* RootComp = OwnerActor->GetRootComponent();
	if (!RootComp) return false;

	const USceneComponent* AttachParent = RootComp->GetAttachParent();
	if (!AttachParent) return false;

	const AActor* AttachParentOwner = AttachParent->GetOwner();
	return AttachParentOwner && AttachParentOwner->IsA(APlatformRoot::StaticClass());
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
