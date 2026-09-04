#include "Level/Lobby/Hub/LobbyHubCharacter.h"

#include "AbilitySystemComponent.h"
#include "Animation/MAAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "P_MA/P_MA.h"
#include "Widget/MAOverHeadStatsGauge.h"

ALobbyHubCharacter::ALobbyHubCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bEnableMinimapCapture = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ALobbyHubCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		// The server may start Arrival before BeginPlay while the world is starting.
		// Preserve authored component state before any arrival phase can detach the Mesh.
		DefaultMeshRelativeTransform = MeshComponent->GetRelativeTransform();
		DefaultMeshCollisionProfileName = MeshComponent->GetCollisionProfileName();
	}
}

void ALobbyHubCharacter::BeginPlay()
{
	Super::BeginPlay();

	// The inherited stimuli source is not part of the Hub character's role.
	if (UAIPerceptionStimuliSourceComponent* StimuliSource = FindComponentByClass<UAIPerceptionStimuliSourceComponent>())
	{
		StimuliSource->UnregisterFromSense(UAISense_Sight::StaticClass());
	}

	// Identify the inherited status gauge by its widget type so other widget components remain untouched.
	TInlineComponentArray<UWidgetComponent*> WidgetComponents(this);
	for (UWidgetComponent* WidgetComponent : WidgetComponents)
	{
		if (Cast<UMAOverHeadStatsGauge>(WidgetComponent->GetUserWidgetObject()))
		{
			WidgetComponent->SetHiddenInGame(true);
			break;
		}
	}

	if (!HasAuthority() && ArrivalState.Phase != ELobbyHubArrivalPhase::Inactive)
	{
		ApplyArrivalPhase();
	}
}

void ALobbyHubCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ArrivalState.Phase == ELobbyHubArrivalPhase::Ragdoll)
	{
		UpdateRagdoll(DeltaSeconds);
	}

	if (bRecoveryMeshBlendActive)
	{
		UpdateRecoveryMeshBlend(DeltaSeconds);
	}
}

void ALobbyHubCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALobbyHubCharacter, ArrivalState);
}

/** Hub Runtime **/
void ALobbyHubCharacter::InitializeHubRuntime()
{
	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent())
	{
		// Do not call ServerSideInit(). The Hub owns no combat stats, effects, or granted abilities.
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		AbilitySystemComponent->SetLooseGameplayTagCount(UMAAbilitySystemStatics::GetAbilityBlockTag(), 1);
		if (!HasAuthority()) return;

		AbilitySystemComponent->SetNumericAttributeBase(UMAAttributeSet::GetMoveSpeedAttribute(), HubMoveSpeed);
		AbilitySystemComponent->SetNumericAttributeBase(UMAAttributeSet::GetSlowMultiplierAttribute(), 1.f);
	}
}

/** Arrival Lifecycle **/
bool ALobbyHubCharacter::BeginArrival(
	const FVector& InitialVelocity,
	const FVector& GroundLocation)
{
	static const FName RagdollRootBoneName(TEXT("pelvis"));
	if (!HasAuthority()
		|| ArrivalState.Phase != ELobbyHubArrivalPhase::Inactive
		|| !GetMesh()
		|| !GetMesh()->GetPhysicsAsset()
		|| GetMesh()->GetBoneIndex(RagdollRootBoneName) == INDEX_NONE)
	{
		return false;
	}

	ArrivalState = FLobbyHubArrivalState();
	ArrivalState.InitialVelocity = InitialVelocity;
	ArrivalGroundLocation = GroundLocation;
	ArrivalElapsedTime = 0.f;
	ArrivalStableTime = 0.f;
	const float GravityMagnitude = FMath::Abs(GetWorld()->GetGravityZ());
	const float UpwardSpeed = FMath::Max(FVector::DotProduct(InitialVelocity, FVector::UpVector), 0.f);
	ArrivalMinimumSettleTime = GravityMagnitude > UE_KINDA_SMALL_NUMBER
		? 2.f * UpwardSpeed / GravityMagnitude
		: 0.f;
	SetArrivalPhase(ELobbyHubArrivalPhase::Ragdoll);
	return true;
}

void ALobbyHubCharacter::BeginRecovery()
{
	if (!HasAuthority() || ArrivalState.Phase != ELobbyHubArrivalPhase::Ragdoll) return;

	const FTransform RecoveryTransform = ResolveRecoveryTransform();
	ArrivalState.RecoveryLocation = RecoveryTransform.GetLocation();
	ArrivalState.RecoveryRotation = RecoveryTransform.Rotator();
	SetArrivalPhase(ELobbyHubArrivalPhase::Recovering);
}

void ALobbyHubCharacter::FinishArrival()
{
	if (!HasAuthority())
	{
		if (ArrivalState.Phase == ELobbyHubArrivalPhase::Inactive)
		{
			SetInputEnabledFromPlayerController(true);
		}
		return;
	}
	if (ArrivalState.Phase == ELobbyHubArrivalPhase::Inactive) return;

	ArrivalState = FLobbyHubArrivalState();
	ArrivalGroundLocation = FVector::ZeroVector;
	ArrivalMinimumSettleTime = 0.f;
	ArrivalElapsedTime = 0.f;
	ArrivalStableTime = 0.f;
	SetArrivalPhase(ELobbyHubArrivalPhase::Inactive);
}

void ALobbyHubCharacter::SetArrivalPhase(const ELobbyHubArrivalPhase NewPhase)
{
	check(HasAuthority());
	ArrivalState.Phase = NewPhase;
	ApplyArrivalPhase();
	ForceNetUpdate();
}

void ALobbyHubCharacter::OnRep_ArrivalState()
{
	if (HasActorBegunPlay())
	{
		ApplyArrivalPhase();
	}
}

void ALobbyHubCharacter::ApplyArrivalPhase()
{
	switch (ArrivalState.Phase)
	{
	case ELobbyHubArrivalPhase::Ragdoll:
		StartRagdoll();
		break;
	case ELobbyHubArrivalPhase::Recovering:
		StartRecovery();
		break;
	case ELobbyHubArrivalPhase::Inactive:
		RestoreHubControl();
		break;
	default:
		checkNoEntry();
	}
}

/** Ragdoll **/
void ALobbyHubCharacter::StartRagdoll()
{
	static const FName RagdollRootBoneName(TEXT("pelvis"));
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent || !ensureMsgf(
		MeshComponent->GetAttachParent() == GetCapsuleComponent()
			&& MeshComponent->GetBoneIndex(RagdollRootBoneName) != INDEX_NONE,
		TEXT("Lobby Hub arrival requires the Character Mesh to be attached to its Capsule.")))
	{
		return;
	}
	bRecoveryMeshBlendActive = false;

	SetInputEnabledFromPlayerController(false);
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.f);
	}
	MeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	MeshComponent->SetCollisionProfileName(TEXT("Ragdoll"));
	MeshComponent->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Ignore);
	MeshComponent->SetAllBodiesSimulatePhysics(true);
	MeshComponent->SetAllPhysicsLinearVelocity(ArrivalState.InitialVelocity);
	MeshComponent->WakeAllRigidBodies();
}

void ALobbyHubCharacter::UpdateRagdoll(const float DeltaSeconds)
{
	static const FName RagdollRootBoneName(TEXT("pelvis"));
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if ((HasAuthority() || IsLocallyControlled())
		&& MeshComponent
		&& MeshComponent->GetBoneIndex(RagdollRootBoneName) != INDEX_NONE)
	{
		SetActorLocation(
			MeshComponent->GetBoneLocation(RagdollRootBoneName, EBoneSpaces::WorldSpace),
			false,
			nullptr,
			ETeleportType::TeleportPhysics);
	}
	if (!HasAuthority()) return;

	ArrivalElapsedTime += DeltaSeconds;
	if (ArrivalElapsedTime >= ArrivalMinimumSettleTime)
	{
		const float RagdollSpeed = MeshComponent->GetPhysicsLinearVelocity(RagdollRootBoneName).Size();
		if (RagdollSpeed <= FMath::Max(ArrivalSettleSpeed, 0.f))
		{
			ArrivalStableTime += DeltaSeconds;
		}
		else
		{
			ArrivalStableTime = 0.f;
		}
	}

	const bool bHasSettled = ArrivalStableTime >= FMath::Max(ArrivalSettleDuration, 0.f);
	const bool bTimedOut = ArrivalElapsedTime
		>= FMath::Max(MaxRagdollArrivalTime, ArrivalMinimumSettleTime);
	if (bHasSettled || bTimedOut)
	{
		BeginRecovery();
	}
}

/** Recovery **/
FTransform ALobbyHubCharacter::ResolveRecoveryTransform() const
{
	static const FName RagdollRootBoneName(TEXT("pelvis"));
	FVector GroundLocation = ArrivalGroundLocation;
	const FVector RagdollLocation = GetMesh()
		? GetMesh()->GetBoneLocation(RagdollRootBoneName, EBoneSpaces::WorldSpace)
		: GetActorLocation();

	FHitResult GroundHit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LobbyHubArrivalRecovery), false, this);
	if (GetWorld()->LineTraceSingleByChannel(
		GroundHit,
		RagdollLocation + FVector::UpVector * 200.f,
		RagdollLocation - FVector::UpVector * 1000.f,
		ECC_Visibility,
		QueryParams))
	{
		GroundLocation = GroundHit.ImpactPoint;
	}

	const float WalkingFloorOffset = (
		UCharacterMovementComponent::MIN_FLOOR_DIST
		+ UCharacterMovementComponent::MAX_FLOOR_DIST) * 0.5f;
	GroundLocation.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + WalkingFloorOffset;
	FRotator RecoveryRotation = GetActorRotation();
	RecoveryRotation.Pitch = 0.f;
	RecoveryRotation.Roll = 0.f;
	return FTransform(RecoveryRotation, GroundLocation);
}

void ALobbyHubCharacter::StartRecovery()
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent) return;

	UpdateArrivalNetworkSmoothing(true);

	UMAAnimInstance* AnimInstance = Cast<UMAAnimInstance>(MeshComponent->GetAnimInstance());
	const FSimpleDelegate RecoveryCompleted = FSimpleDelegate::CreateUObject(
		this, &ALobbyHubCharacter::FinishArrival);
	const bool bRecoveryStarted = AnimInstance
		&& AnimInstance->RecoverPose(RecoveryCompleted);

	MeshComponent->SetAllBodiesSimulatePhysics(false);
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetActorLocationAndRotation(
		ArrivalState.RecoveryLocation,
		ArrivalState.RecoveryRotation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);

	// Keep the final simulated Mesh alignment until its transform blend catches up.
	if (MeshComponent->GetAttachParent() != GetCapsuleComponent())
	{
		MeshComponent->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepWorldTransform);
	}
	BeginRecoveryMeshBlend();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	if (!bRecoveryStarted && HasAuthority())
	{
		FinishArrival();
	}
}

void ALobbyHubCharacter::BeginRecoveryMeshBlend()
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent) return;

	RecoveryMeshStartRelativeTransform = MeshComponent->GetRelativeTransform();
	const float SafeBlendDuration = FMath::Max(ArrivalMeshBlendDuration, 0.f);
	if (SafeBlendDuration <= UE_KINDA_SMALL_NUMBER)
	{
		MeshComponent->SetRelativeTransform(DefaultMeshRelativeTransform);
		bRecoveryMeshBlendActive = false;
		return;
	}

	RecoveryMeshBlend.SetBlendTime(SafeBlendDuration);
	RecoveryMeshBlend.SetBlendOption(EAlphaBlendOption::HermiteCubic);
	RecoveryMeshBlend.SetValueRange(0.f, 1.f);
	RecoveryMeshBlend.Reset();
	bRecoveryMeshBlendActive = true;
}

void ALobbyHubCharacter::UpdateRecoveryMeshBlend(const float DeltaSeconds)
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		bRecoveryMeshBlendActive = false;
		return;
	}

	RecoveryMeshBlend.Update(DeltaSeconds);
	FTransform BlendedRelativeTransform;
	BlendedRelativeTransform.Blend(
		RecoveryMeshStartRelativeTransform,
		DefaultMeshRelativeTransform,
		RecoveryMeshBlend.GetBlendedValue());
	MeshComponent->SetRelativeTransform(
		BlendedRelativeTransform,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
	if (!RecoveryMeshBlend.IsComplete()) return;

	MeshComponent->SetRelativeTransform(DefaultMeshRelativeTransform);
	bRecoveryMeshBlendActive = false;
}

void ALobbyHubCharacter::UpdateArrivalNetworkSmoothing(const bool bIsRecovering)
{
	if (IsLocallyControlled()) return;

	const ENetMode NetMode = GetNetMode();
	if (NetMode != NM_Client && NetMode != NM_ListenServer) return;

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (bIsRecovering)
	{
		if (!bHasSavedArrivalNetworkSmoothingMode)
		{
			SavedArrivalNetworkSmoothingMode = MovementComponent->NetworkSmoothingMode;
			bHasSavedArrivalNetworkSmoothingMode = true;
		}
		MovementComponent->NetworkSmoothingMode = ENetworkSmoothingMode::Disabled;
		return;
	}

	if (bHasSavedArrivalNetworkSmoothingMode)
	{
		MovementComponent->NetworkSmoothingMode = SavedArrivalNetworkSmoothingMode;
		bHasSavedArrivalNetworkSmoothingMode = false;
	}
}

void ALobbyHubCharacter::RestoreHubControl()
{
	bRecoveryMeshBlendActive = false;

	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		// Replication may skip an intermediate phase, so the final state closes every Ragdoll concern.
		MeshComponent->SetAllBodiesSimulatePhysics(false);
		MeshComponent->SetSimulatePhysics(false);
		if (!DefaultMeshCollisionProfileName.IsNone())
		{
			MeshComponent->SetCollisionProfileName(DefaultMeshCollisionProfileName);
		}
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (MeshComponent->GetAttachParent() != GetCapsuleComponent())
		{
			MeshComponent->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepWorldTransform);
		}
		MeshComponent->SetRelativeTransform(DefaultMeshRelativeTransform);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	const UMAAnimInstance* AnimInstance = Cast<UMAAnimInstance>(GetMesh()->GetAnimInstance());
	if (!AnimInstance || !AnimInstance->IsPoseRecoveryActive())
	{
		SetInputEnabledFromPlayerController(true);
	}
	UpdateArrivalNetworkSmoothing(false);
}
