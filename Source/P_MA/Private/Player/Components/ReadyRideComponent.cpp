#include "Player/Components/ReadyRideComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Level/Platform/RideRoot.h"
#include "Net/UnrealNetwork.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/Components/MAPlayerCharacterMovementComponent.h"

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

	CacheOwnerMeshAttachment();

	if (const AActor* OwnerActor = GetOwner())
	{
		PrevTickLocation = OwnerActor->GetActorLocation();
	}
	ApplyRideState(IsRiding());
}

void UReadyRideComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AMAPlayerCharacter* OwnerCharacter = Cast<AMAPlayerCharacter>(GetOwner());
	if (!OwnerCharacter) return;

	const AActor* OwnerActor = OwnerCharacter;

	const bool bIsRiding = IsRiding();

	const FVector CurrentLocation = OwnerActor->GetActorLocation();
	if (bIsRiding && DeltaTime > KINDA_SMALL_NUMBER)
	{
		FVector Delta = CurrentLocation - PrevTickLocation;
		Delta.Z = 0.f;
		RideMoveVelocity = Delta / DeltaTime;
		RideMoveSpeed = RideMoveVelocity.Size();
	}
	else
	{
		RideMoveVelocity = FVector::ZeroVector;
		RideMoveSpeed = 0.f;
	}

	PrevTickLocation = CurrentLocation;
}

void UReadyRideComponent::SetRidingRoot(ARideRoot* InRideRoot)
{
	RidingRoot = InRideRoot;
	if (!RidingRoot)
	{
		RideMoveVelocity = FVector::ZeroVector;
		RideMoveSpeed = 0.f;
	}

	if (const AActor* OwnerActor = GetOwner())
	{
		PrevTickLocation = OwnerActor->GetActorLocation();
	}

	ApplyRideState(IsRiding());
}

bool UReadyRideComponent::TryGetRideYaw(float& OutYaw) const
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

	const bool bNowRiding = IsRiding();
	if (bNowRiding == bIsRidingState) return;
	bIsRidingState = bNowRiding;

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
		RideMoveVelocity = FVector::ZeroVector;
		RideMoveSpeed = 0.f;
	}
	ApplyRideState(IsRiding());
}

void UReadyRideComponent::SyncRideMovementBase() const
{
	AMAPlayerCharacter* OwnerCharacter = Cast<AMAPlayerCharacter>(GetOwner());
	if (!OwnerCharacter) return;

	UPrimitiveComponent* CurrentBase = OwnerCharacter->GetMovementBase();
	UPrimitiveComponent* NewBase = RidingRoot ? RidingRoot->GetRideBaseComponent() : nullptr;
	if (CurrentBase == NewBase) return;

	if (NewBase)
	{
		OwnerCharacter->SetBase(NewBase);
		return;
	}

	if (CurrentBase && CurrentBase->GetOwner() && CurrentBase->GetOwner()->IsA<ARideRoot>())
	{
		OwnerCharacter->SetBase(nullptr);
	}
}

void UReadyRideComponent::ApplyRideState(bool bIsRiding)
{
	UpdateRideMovementMode(bIsRiding);
	SyncRideMovementBase();
	UpdateMountState(bIsRiding);
	UpdateRemoteViewMeshSmoothing(bIsRiding);
	UpdateTickPolicy(bIsRiding);
	RefreshRideCollisionMode();
}

void UReadyRideComponent::UpdateRideMovementMode(bool bIsRiding) const
{
	AMAPlayerCharacter* OwnerCharacter = Cast<AMAPlayerCharacter>(GetOwner());
	if (!OwnerCharacter) return;

	UMAPlayerCharacterMovementComponent* MoveComp = CastChecked<UMAPlayerCharacterMovementComponent>(OwnerCharacter->GetCharacterMovement());
	MoveComp->SetRideMovementEnabled(bIsRiding);
}

void UReadyRideComponent::UpdateTickPolicy(bool bIsRiding)
{
	if (bIsRiding)
	{
		PrimaryComponentTick.TickInterval = 0.f;
		SetComponentTickEnabled(true);
		return;
	}

	SetComponentTickEnabled(false);
}

void UReadyRideComponent::UpdateMountState(bool bIsRiding)
{
	const ERideMountState NewMountState = bIsRiding ? ERideMountState::Mounted : ERideMountState::None;
	if (MountState == NewMountState) return;

	MountState = NewMountState;
	if (MountState == ERideMountState::Mounted)
	{
		AttachOwnerMeshToMount();
		return;
	}

	RestoreOwnerMeshAttachment();
}

void UReadyRideComponent::UpdateRemoteViewMeshSmoothing(bool bIsRiding)
{
	AMAPlayerCharacter* OwnerCharacter = Cast<AMAPlayerCharacter>(GetOwner());
	if (!OwnerCharacter) return;
	if (OwnerCharacter->IsLocallyControlled()) return;

	const ENetMode NetMode = OwnerCharacter->GetNetMode();
	if (NetMode != NM_Client && NetMode != NM_ListenServer) return;

	UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
	if (bIsRiding && MountState == ERideMountState::Mounted)
	{
		if (!bHasSavedProxyNetworkSmoothingMode)
		{
			SavedProxyNetworkSmoothingMode = MoveComp->NetworkSmoothingMode;
			bHasSavedProxyNetworkSmoothingMode = true;
		}

		if (MoveComp->NetworkSmoothingMode != ENetworkSmoothingMode::Disabled)
		{
			MoveComp->NetworkSmoothingMode = ENetworkSmoothingMode::Disabled;
		}
		return;
	}

	if (bHasSavedProxyNetworkSmoothingMode)
	{
		MoveComp->NetworkSmoothingMode = SavedProxyNetworkSmoothingMode;
		bHasSavedProxyNetworkSmoothingMode = false;
	}
}

void UReadyRideComponent::CacheOwnerMeshAttachment()
{
	if (bHasCachedOwnerMeshAttachment) return;

	const AMAPlayerCharacter* OwnerCharacter = Cast<AMAPlayerCharacter>(GetOwner());
	if (!OwnerCharacter) return;

	const USkeletalMeshComponent* OwnerMesh = OwnerCharacter->GetMesh();
	if (!OwnerMesh) return;

	CachedOwnerMeshParent = OwnerMesh->GetAttachParent();
	CachedOwnerMeshRelativeTransform = OwnerMesh->GetRelativeTransform();
	bHasCachedOwnerMeshAttachment = CachedOwnerMeshParent.IsValid();
}

void UReadyRideComponent::AttachOwnerMeshToMount()
{
	AMAPlayerCharacter* OwnerCharacter = Cast<AMAPlayerCharacter>(GetOwner());
	if (!OwnerCharacter) return;

	USkeletalMeshComponent* OwnerMesh = OwnerCharacter->GetMesh();
	USkeletalMeshComponent* MountMesh = OwnerCharacter->GetMountMesh();
	if (!OwnerMesh || !MountMesh) return;

	CacheOwnerMeshAttachment();
	MountMesh->SetHiddenInGame(false);
	OwnerMesh->AttachToComponent(MountMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, MountSocketName);
}

void UReadyRideComponent::RestoreOwnerMeshAttachment()
{
	AMAPlayerCharacter* OwnerCharacter = Cast<AMAPlayerCharacter>(GetOwner());
	if (!OwnerCharacter) return;

	USkeletalMeshComponent* OwnerMesh = OwnerCharacter->GetMesh();
	USkeletalMeshComponent* MountMesh = OwnerCharacter->GetMountMesh();
	if (!OwnerMesh) return;

	USceneComponent* RestoreParent = CachedOwnerMeshParent.Get();
	if (!RestoreParent)
	{
		RestoreParent = OwnerCharacter->GetRootComponent();
	}

	if (RestoreParent)
	{
		OwnerMesh->AttachToComponent(RestoreParent, FAttachmentTransformRules::KeepRelativeTransform);
		OwnerMesh->SetRelativeTransform(CachedOwnerMeshRelativeTransform);
	}

	if (MountMesh)
	{
		MountMesh->SetHiddenInGame(true);
	}
}

void UReadyRideComponent::UpdateRideCollisionWithOtherPlayer(AMAPlayerCharacter* OwnerCharacter, AMAPlayerCharacter* OtherPlayer) const
{
	if (!OwnerCharacter || !OtherPlayer) return;

	UCapsuleComponent* Capsule = OwnerCharacter->GetCapsuleComponent();
	UCapsuleComponent* OtherCapsule = OtherPlayer->GetCapsuleComponent();
	if (!Capsule || !OtherCapsule) return;

	const UReadyRideComponent* OtherReadyRideComp = OtherPlayer->FindComponentByClass<UReadyRideComponent>();
	const bool bOtherRiding = OtherReadyRideComp ? OtherReadyRideComp->bIsRidingState : false;
	const bool bShouldIgnorePair = bIsRidingState || bOtherRiding;

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
