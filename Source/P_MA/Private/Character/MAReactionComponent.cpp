#include "Character/MAReactionComponent.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Character/MACharacter.h"
#include "Components/CapsuleComponent.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "P_MA/P_MA.h"

namespace
{
	constexpr int32 DefaultCrowdControlBlockFlags =
		static_cast<int32>(EReactionBlockFlags::Move) |
		static_cast<int32>(EReactionBlockFlags::Rotation) |
		static_cast<int32>(EReactionBlockFlags::Ability);

	FReactionRule MakeReactionRule(
		const FGameplayTag& CrowdControlTag,
		const EReactionImpulseMode ImpulseMode,
		const bool bStopAIOnStart,
		const bool bStopMovementOnStart)
	{
		FReactionRule ReactionRule;
		ReactionRule.CrowdControlTag = CrowdControlTag;
		ReactionRule.BlockFlags = DefaultCrowdControlBlockFlags;
		ReactionRule.ImpulseMode = ImpulseMode;
		ReactionRule.bPlayMontageOnStart = true;
		ReactionRule.bStopMontageOnEnd = true;
		ReactionRule.bStopAIOnStart = bStopAIOnStart;
		ReactionRule.bStopMovementOnStart = bStopMovementOnStart;
		return ReactionRule;
	}

	void AddDefaultReactionRules(TArray<FReactionRule>& ReactionRules)
	{
		if (ReactionRules.Num() > 0) return;

		ReactionRules.Add(MakeReactionRule(
			UMAAbilitySystemStatics::GetAirborneStatTag(),
			EReactionImpulseMode::None,
			true,
			true));

		ReactionRules.Add(MakeReactionRule(
			UMAAbilitySystemStatics::GetStunStatTag(),
			EReactionImpulseMode::None,
			true,
			true));

		ReactionRules.Add(MakeReactionRule(
			UMAAbilitySystemStatics::GetGrabStatTag(),
			EReactionImpulseMode::PullToSource,
			false,
			false));

		ReactionRules.Add(MakeReactionRule(
			UMAAbilitySystemStatics::GetKnockbackStatTag(),
			EReactionImpulseMode::PushFromSource,
			false,
			false));

		ReactionRules.Add(MakeReactionRule(
			UMAAbilitySystemStatics::GetStaggerStatTag(),
			EReactionImpulseMode::PushFromSource,
			true,
			true));
	}
}

UMAReactionComponent::UMAReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.SetTickFunctionEnable(false);
}

void UMAReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AMACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		if (USkeletalMeshComponent* MeshComponent = OwnerCharacter->GetMesh())
		{
			BaseAirborneMeshRelativeLocation = MeshComponent->GetRelativeLocation();
		}
	}
	BindToASC();
}

void UMAReactionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ResetTransientReactionState();
	Super::EndPlay(EndPlayReason);
}

void UMAReactionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateAirborneVisual(DeltaTime);
}

void UMAReactionComponent::BuildReactionRules()
{
	ReactionRules.Reset();
	AddDefaultReactionRules(ReactionRules);
}

const FReactionRule* UMAReactionComponent::FindReactionRule(const FGameplayTag& ReactionTag) const
{
	for (const FReactionRule& ReactionRule : ReactionRules)
	{
		if (ReactionRule.CrowdControlTag == ReactionTag)
		{
			return &ReactionRule;
		}
	}
	return nullptr;
}

bool UMAReactionComponent::HasActiveImpulseReaction() const
{
	return ActiveImpulseContributions.Num() > 0;
}

void UMAReactionComponent::BindToASC()
{
	if (!OwnerCharacter) return;

	OwnerASC = Cast<UMAAbilitySystemComponent>(OwnerCharacter->GetAbilitySystemComponent());
	if (!OwnerASC) return;
	BuildReactionRules();
	if (ReactionRules.Num() == 0) return;

	for (const FReactionRule& ReactionRule : ReactionRules)
	{
		if (!ReactionRule.IsValid()) continue;
		OwnerASC->RegisterGameplayTagEvent(ReactionRule.CrowdControlTag).AddUObject(this, &UMAReactionComponent::HandleCrowdControlChanged);
	}

	OwnerASC->OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UMAReactionComponent::HandleCrowdControlApplied);
}

/*************************************************************/
/*					CrowdControl Changed					 */
/*************************************************************/
void UMAReactionComponent::HandleCrowdControlChanged(FGameplayTag Tag, int32 NewCount)
{
	const FReactionRule* ReactionRule = FindReactionRule(Tag);
	if (!ReactionRule) return;

	if (NewCount > 0)
	{
		const bool bWasAdded = !ActiveCrowdControlTags.Contains(Tag);
		ActiveCrowdControlTags.Add(Tag);
		if (bWasAdded)
		{
			HandleCrowdControlStarted(*ReactionRule);
		}
		return;
	}

	if (ActiveCrowdControlTags.Remove(Tag) > 0)
	{
		HandleCrowdControlEnded(*ReactionRule);
	}
}

void UMAReactionComponent::HandleCrowdControlStarted(const FReactionRule& ReactionRule)
{
	if (!OwnerCharacter || OwnerCharacter->IsDead()) return;

	if (ReactionRule.bPlayMontageOnStart)
	{
		FReactionAnimConfig ReactionAnimConfig;
		if (GetReactionAnimConfig(ReactionRule.CrowdControlTag, ReactionAnimConfig) && ReactionAnimConfig.Montage)
		{
			OwnerCharacter->PlayAnimMontage(ReactionAnimConfig.Montage);
		}
	}

	if (ReactionRule.bStopAIOnStart)
	{
		if (AAIController* AIController = Cast<AAIController>(OwnerCharacter->GetController()))
		{
			AIController->StopMovement();
		}
	}

	if (ReactionRule.bStopMovementOnStart)
	{
		if (UCharacterMovementComponent* CharacterMovementComponent = OwnerCharacter->GetCharacterMovement())
		{
			CharacterMovementComponent->StopMovementImmediately();
		}
	}

	if (ReactionRule.CrowdControlTag == UMAAbilitySystemStatics::GetAirborneStatTag())
	{
		BeginAirborneVisual();
	}

	RefreshControlBlockTags();
}

void UMAReactionComponent::HandleCrowdControlEnded(const FReactionRule& ReactionRule)
{
	if (ReactionRule.bStopMontageOnEnd)
	{
		StopReactionMontage(ReactionRule.CrowdControlTag);
	}

	if (ReactionRule.CrowdControlTag == UMAAbilitySystemStatics::GetAirborneStatTag())
	{
		EndAirborneVisual();
	}

	if (ReactionRule.ImpulseMode != EReactionImpulseMode::None)
	{
		ActiveImpulseContributions.Remove(ReactionRule.CrowdControlTag);
		if (HasActiveImpulseReaction())
		{
			RecalculateImpulseReactionVelocity(false);
		}
		else
		{
			ClearImpulseReactionState();
		}
	}

	RefreshControlBlockTags();
}

/*************************************************************/
/*					CrowdControl Applied					 */
/*************************************************************/
void UMAReactionComponent::HandleCrowdControlApplied(UAbilitySystemComponent* SourceASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle ActiveHandle)
{
	(void)SourceASC;
	(void)ActiveHandle;

	if (!OwnerCharacter || !OwnerCharacter->HasAuthority() || !OwnerASC) return;

	HandleAirborneCrowdControlApplied(Spec);
	HandleImpulseCrowdControlApplied(Spec);
}

void UMAReactionComponent::HandleAirborneCrowdControlApplied(const FGameplayEffectSpec& Spec)
{
	const FGameplayTag AirborneTag = UMAAbilitySystemStatics::GetAirborneStatTag();
	if (!Spec.DynamicGrantedTags.HasTag(AirborneTag)) return;

	USkeletalMeshComponent* MeshComponent = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;
	if (!MeshComponent) return;

	const float AppliedHeight = Spec.GetSetByCallerMagnitude(AirborneTag, false, 0.f);
	CurrentAirborneVisualHeight = !FMath::IsNearlyZero(AppliedHeight) ? AppliedHeight : AirborneVisualHeight;
	CurrentAirborneVisualDuration = FMath::Max(Spec.GetDuration(), 0.f);

	const float AppliedRiseTime = Spec.GetSetByCallerMagnitude(UMAAbilitySystemStatics::GetAirborneRiseTimeTag(), false, 0.f);
	if (CurrentAirborneVisualDuration > 0.f)
	{
		CurrentAirborneVisualRiseTime = AppliedRiseTime > 0.f ? FMath::Min(AppliedRiseTime, CurrentAirborneVisualDuration) : CurrentAirborneVisualDuration * 0.5f;
	}
	else
	{
		CurrentAirborneVisualRiseTime = 0.f;
	}

	AirborneVisualStartMeshRelativeLocation = bAirborneVisualActive
		? MeshComponent->GetRelativeLocation()
		: BaseAirborneMeshRelativeLocation;
	AirborneVisualPeakMeshRelativeLocation = BaseAirborneMeshRelativeLocation;
	AirborneVisualPeakMeshRelativeLocation.Z = FMath::Max(
		AirborneVisualStartMeshRelativeLocation.Z,
		BaseAirborneMeshRelativeLocation.Z + CurrentAirborneVisualHeight);
	CurrentAirborneVisualElapsedTime = 0.f;

	if (bAirborneVisualActive)
	{
		SetComponentTickEnabled(true);
	}
}

void UMAReactionComponent::HandleImpulseCrowdControlApplied(const FGameplayEffectSpec& Spec)
{
	for (const FReactionRule& ReactionRule : ReactionRules)
	{
		if (ReactionRule.ImpulseMode == EReactionImpulseMode::None) continue;
		if (!Spec.DynamicGrantedTags.HasTag(ReactionRule.CrowdControlTag)) continue;

		const float Magnitude = Spec.GetSetByCallerMagnitude(ReactionRule.CrowdControlTag, false, 0.f);
		if (Magnitude <= 0.f) continue;

		FVector SourcePoint = FVector::ZeroVector;
		if (!UMAAbilitySystemStatics::TryGetReactionSourcePoint(Spec, SourcePoint))
		{
			const AActor* InstigatorActor = Spec.GetContext().GetInstigator();
			SourcePoint = InstigatorActor ? InstigatorActor->GetActorLocation() : OwnerCharacter->GetActorLocation();
		}

		ApplyImpulseReaction(ReactionRule.ImpulseMode, Magnitude, SourcePoint, ReactionRule.CrowdControlTag);
	}
}

void UMAReactionComponent::ApplyImpulseReaction(EReactionImpulseMode ImpulseMode, float Magnitude, const FVector& SourcePoint, const FGameplayTag& ReactionTag)
{
	if (!OwnerCharacter || OwnerCharacter->IsDead() || Magnitude <= 0.f) return;

	FReactionAnimConfig ReactionAnimConfig;
	float VerticalLaunchScale = 0.f;
	if (GetReactionAnimConfig(ReactionTag, ReactionAnimConfig))
	{
		VerticalLaunchScale = ReactionAnimConfig.VerticalLaunchScale;
	}

	FVector ReactionDirection = GetReactionDirection(SourcePoint, ImpulseMode);
	if (!FMath::IsNearlyZero(VerticalLaunchScale))
	{
		ReactionDirection.Z = VerticalLaunchScale;
	}

	if (UCharacterMovementComponent* CharacterMovementComponent = OwnerCharacter->GetCharacterMovement())
	{
		if (AAIController* AIController = Cast<AAIController>(OwnerCharacter->GetController()))
		{
			AIController->StopMovement();
		}

		const bool bShouldStopMovementImmediately = !bImpulseMovementOverrideActive;
		BeginImpulseMovementOverride();
		ActiveImpulseContributions.Add(ReactionTag, ReactionDirection * Magnitude);
		RecalculateImpulseReactionVelocity(bShouldStopMovementImmediately);
	}
}

/*************************************************************/
/*						Impulse							 */
/*************************************************************/
void UMAReactionComponent::BeginImpulseMovementOverride()
{
	if (!OwnerCharacter || OwnerCharacter->IsDead()) return;

	UCharacterMovementComponent* Move = OwnerCharacter->GetCharacterMovement();
	UCapsuleComponent* Capsule = OwnerCharacter->GetCapsuleComponent();
	if (!Move || !Capsule) return;

	if (!bImpulseMovementOverrideActive)
	{
		SavedImpulseGroundFriction = Move->GroundFriction;
		SavedImpulseBrakingFrictionFactor = Move->BrakingFrictionFactor;
		SavedImpulseBrakingDecelerationWalking = Move->BrakingDecelerationWalking;
		SavedImpulseHitboxResponse = Capsule->GetCollisionResponseToChannel(ECC_Hitbox);
		bImpulseMovementOverrideActive = true;
	}

	Move->GroundFriction = 0.f;
	Move->BrakingFrictionFactor = 0.f;
	Move->BrakingDecelerationWalking = 0.f;
	Capsule->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Overlap);
}

void UMAReactionComponent::EndImpulseMovementOverride()
{
	if (!OwnerCharacter || !bImpulseMovementOverrideActive) return;

	if (UCharacterMovementComponent* Move = OwnerCharacter->GetCharacterMovement())
	{
		Move->GroundFriction = SavedImpulseGroundFriction;
		Move->BrakingFrictionFactor = SavedImpulseBrakingFrictionFactor;
		Move->BrakingDecelerationWalking = SavedImpulseBrakingDecelerationWalking;
	}

	if (UCapsuleComponent* Capsule = OwnerCharacter->GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_Hitbox, SavedImpulseHitboxResponse);
	}

	bImpulseMovementOverrideActive = false;
}

void UMAReactionComponent::StopReactionMontage(const FGameplayTag& ReactionTag)
{
	if (!OwnerCharacter) return;

	FReactionAnimConfig ReactionAnimConfig;
	if (GetReactionAnimConfig(ReactionTag, ReactionAnimConfig) && ReactionAnimConfig.Montage)
	{
		OwnerCharacter->StopAnimMontage(ReactionAnimConfig.Montage);
	}
}

void UMAReactionComponent::StopAllReactionMontages()
{
	for (const FReactionRule& ReactionRule : ReactionRules)
	{
		if (!ReactionRule.IsValid()) continue;
		StopReactionMontage(ReactionRule.CrowdControlTag);
	}
}

void UMAReactionComponent::ClearImpulseReactionState()
{
	ActiveImpulseContributions.Reset();

	if (OwnerCharacter)
	{
		if (UCharacterMovementComponent* CharacterMovementComponent = OwnerCharacter->GetCharacterMovement())
		{
			CharacterMovementComponent->Velocity = FVector::ZeroVector;
		}
	}

	if (bImpulseMovementOverrideActive)
	{
		EndImpulseMovementOverride();
	}
}

void UMAReactionComponent::RecalculateImpulseReactionVelocity(bool bStopMovementImmediately)
{
	if (!OwnerCharacter) return;

	UCharacterMovementComponent* CharacterMovementComponent = OwnerCharacter->GetCharacterMovement();
	if (!CharacterMovementComponent) return;

	FVector CombinedVelocity = FVector::ZeroVector;
	for (const TPair<FGameplayTag, FVector>& ContributionEntry : ActiveImpulseContributions)
	{
		CombinedVelocity += ContributionEntry.Value;
	}

	if (bStopMovementImmediately)
	{
		CharacterMovementComponent->StopMovementImmediately();
	}

	if (CombinedVelocity.Z > 0.f && CharacterMovementComponent->IsMovingOnGround())
	{
		CharacterMovementComponent->SetMovementMode(MOVE_Falling);
	}

	CharacterMovementComponent->Velocity = CombinedVelocity;
}

void UMAReactionComponent::RefreshControlBlockTags()
{
	if (!OwnerASC) return;

	int32 BlockMask = static_cast<int32>(EReactionBlockFlags::None);
	for (const FGameplayTag& ActiveCrowdControlTag : ActiveCrowdControlTags)
	{
		const FReactionRule* ReactionRule = FindReactionRule(ActiveCrowdControlTag);
		if (!ReactionRule) continue;
		BlockMask |= ReactionRule->BlockFlags;
	}

	const bool bShouldBlockMove = (BlockMask & static_cast<int32>(EReactionBlockFlags::Move)) != 0;
	const bool bShouldBlockRotation = (BlockMask & static_cast<int32>(EReactionBlockFlags::Rotation)) != 0;
	const bool bShouldBlockAbility = (BlockMask & static_cast<int32>(EReactionBlockFlags::Ability)) != 0;

	if (bShouldBlockMove) OwnerASC->AddReplicatedLooseGameplayTag(UMAAbilitySystemStatics::GetMoveBlockTag());
	else OwnerASC->RemoveReplicatedLooseGameplayTag(UMAAbilitySystemStatics::GetMoveBlockTag());

	if (bShouldBlockRotation) OwnerASC->AddReplicatedLooseGameplayTag(UMAAbilitySystemStatics::GetRotationLockTag());
	else OwnerASC->RemoveReplicatedLooseGameplayTag(UMAAbilitySystemStatics::GetRotationLockTag());

	if (bShouldBlockAbility) OwnerASC->AddReplicatedLooseGameplayTag(UMAAbilitySystemStatics::GetAbilityBlockTag());
	else OwnerASC->RemoveReplicatedLooseGameplayTag(UMAAbilitySystemStatics::GetAbilityBlockTag());
}

void UMAReactionComponent::BeginAirborneVisual()
{
	if (!OwnerCharacter || bAirborneVisualActive) return;

	USkeletalMeshComponent* MeshComponent = OwnerCharacter->GetMesh();
	if (!MeshComponent) return;

	AirborneVisualStartMeshRelativeLocation = MeshComponent->GetRelativeLocation();
	AirborneVisualPeakMeshRelativeLocation = BaseAirborneMeshRelativeLocation;
	AirborneVisualPeakMeshRelativeLocation.Z = FMath::Max(
		AirborneVisualStartMeshRelativeLocation.Z,
		BaseAirborneMeshRelativeLocation.Z + CurrentAirborneVisualHeight);
	bAirborneVisualActive = true;
	CurrentAirborneVisualElapsedTime = 0.f;
	SetComponentTickEnabled(true);
}

void UMAReactionComponent::EndAirborneVisual()
{
	if (!OwnerCharacter || !bAirborneVisualActive) return;

	if (USkeletalMeshComponent* MeshComponent = OwnerCharacter->GetMesh())
	{
		MeshComponent->SetRelativeLocation(BaseAirborneMeshRelativeLocation);
	}

	bAirborneVisualActive = false;
	CurrentAirborneVisualHeight = 0.f;
	CurrentAirborneVisualDuration = 0.f;
	CurrentAirborneVisualRiseTime = 0.f;
	CurrentAirborneVisualElapsedTime = 0.f;
	AirborneVisualStartMeshRelativeLocation = BaseAirborneMeshRelativeLocation;
	AirborneVisualPeakMeshRelativeLocation = BaseAirborneMeshRelativeLocation;
	SetComponentTickEnabled(false);
}

void UMAReactionComponent::UpdateAirborneVisual(float DeltaTime)
{
	if (!OwnerCharacter || !bAirborneVisualActive)
	{
		SetComponentTickEnabled(false);
		return;
	}

	USkeletalMeshComponent* MeshComponent = OwnerCharacter->GetMesh();
	if (!MeshComponent)
	{
		SetComponentTickEnabled(false);
		return;
	}

	CurrentAirborneVisualElapsedTime = FMath::Min(CurrentAirborneVisualElapsedTime + DeltaTime, CurrentAirborneVisualDuration);

	const float ClampedRiseTime = FMath::Clamp(CurrentAirborneVisualRiseTime, 0.f, CurrentAirborneVisualDuration);
	FVector NextLocation = BaseAirborneMeshRelativeLocation;

	if (CurrentAirborneVisualElapsedTime <= ClampedRiseTime && ClampedRiseTime > KINDA_SMALL_NUMBER)
	{
		const float RiseAlpha = FMath::Clamp(CurrentAirborneVisualElapsedTime / ClampedRiseTime, 0.f, 1.f);
		const float EasedRiseAlpha = 1.f - FMath::Square(1.f - RiseAlpha);
		NextLocation = FMath::Lerp(AirborneVisualStartMeshRelativeLocation, AirborneVisualPeakMeshRelativeLocation, EasedRiseAlpha);
	}
	else if (CurrentAirborneVisualDuration > KINDA_SMALL_NUMBER)
	{
		const float FallDuration = FMath::Max(CurrentAirborneVisualDuration - ClampedRiseTime, KINDA_SMALL_NUMBER);
		const float FallAlpha = FMath::Clamp((CurrentAirborneVisualElapsedTime - ClampedRiseTime) / FallDuration, 0.f, 1.f);
		const float EasedFallAlpha = FMath::Square(FallAlpha);
		NextLocation = FMath::Lerp(AirborneVisualPeakMeshRelativeLocation, BaseAirborneMeshRelativeLocation, EasedFallAlpha);
	}

	MeshComponent->SetRelativeLocation(NextLocation);

	if (CurrentAirborneVisualElapsedTime >= CurrentAirborneVisualDuration)
	{
		MeshComponent->SetRelativeLocation(BaseAirborneMeshRelativeLocation);
		SetComponentTickEnabled(false);
	}
}

FVector UMAReactionComponent::GetReactionDirection(const FVector& SourcePoint, EReactionImpulseMode ImpulseMode) const
{
	if (!OwnerCharacter) return FVector::ZeroVector;

	FVector ReactionDirection = FVector::ZeroVector;
	switch (ImpulseMode)
	{
	case EReactionImpulseMode::PushFromSource:
		ReactionDirection = (OwnerCharacter->GetActorLocation() - SourcePoint).GetSafeNormal();
		break;
	case EReactionImpulseMode::PullToSource:
		ReactionDirection = (SourcePoint - OwnerCharacter->GetActorLocation()).GetSafeNormal();
		break;
	default:
		return FVector::ZeroVector;
	}

	ReactionDirection.Z = 0.f;
	return ReactionDirection;
}

bool UMAReactionComponent::GetReactionAnimConfig(const FGameplayTag& ReactionTag, FReactionAnimConfig& OutConfig) const
{
	if (const FReactionAnimConfig* FoundConfig = ReactionAnimMap.Find(ReactionTag))
	{
		OutConfig = *FoundConfig;
		return true;
	}

	return false;
}

void UMAReactionComponent::ResetTransientReactionState()
{
	StopAllReactionMontages();
	EndAirborneVisual();
	ClearImpulseReactionState();
	ActiveCrowdControlTags.Reset();
	RefreshControlBlockTags();
}
