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
	void AddDefaultReactionRules(TArray<FReactionRule>& ReactionRules)
	{
		if (ReactionRules.Num() > 0) return;

		FReactionRule StunRule;
		StunRule.CrowdControlTag = UMAAbilitySystemStatics::GetStunStatTag();
		StunRule.BlockFlags = static_cast<int32>(EReactionBlockFlags::Move) |
			static_cast<int32>(EReactionBlockFlags::Rotation) |
			static_cast<int32>(EReactionBlockFlags::Ability);
		StunRule.ImpulseMode = EReactionImpulseMode::None;
		StunRule.bPlayMontageOnStart = true;
		StunRule.bStopMontageOnEnd = true;
		StunRule.bStopAIOnStart = true;
		StunRule.bStopMovementOnStart = true;
		ReactionRules.Add(StunRule);

		FReactionRule KnockbackRule;
		KnockbackRule.CrowdControlTag = UMAAbilitySystemStatics::GetKnockbackStatTag();
		KnockbackRule.BlockFlags = static_cast<int32>(EReactionBlockFlags::Move) |
			static_cast<int32>(EReactionBlockFlags::Rotation) |
			static_cast<int32>(EReactionBlockFlags::Ability);
		KnockbackRule.ImpulseMode = EReactionImpulseMode::PushFromSource;
		KnockbackRule.bPlayMontageOnStart = true;
		KnockbackRule.bStopMontageOnEnd = true;
		ReactionRules.Add(KnockbackRule);
	}
}

UMAReactionComponent::UMAReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMAReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AMACharacter>(GetOwner());
	BindToASC();
}

void UMAReactionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ResetTransientReactionState();
	Super::EndPlay(EndPlayReason);
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
	for (const FGameplayTag& ActiveCrowdControlTag : ActiveCrowdControlTags)
	{
		const FReactionRule* ReactionRule = FindReactionRule(ActiveCrowdControlTag);
		if (!ReactionRule) continue;
		if (ReactionRule->ImpulseMode != EReactionImpulseMode::None)
		{
			return true;
		}
	}
	return false;
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

	if (ReactionRule.ImpulseMode != EReactionImpulseMode::None)
	{
		BeginImpulseMovementOverride();
	}

	RefreshControlBlockTags();
}

void UMAReactionComponent::HandleCrowdControlEnded(const FReactionRule& ReactionRule)
{
	if (ReactionRule.bStopMontageOnEnd)
	{
		StopReactionMontage(ReactionRule.CrowdControlTag);
	}

	if (ReactionRule.ImpulseMode != EReactionImpulseMode::None && !HasActiveImpulseReaction())
	{
		ClearImpulseReactionState();
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

	HandleImpulseCrowdControlApplied(Spec);
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
		if (ReactionAnimConfig.Montage)
		{
			OwnerCharacter->PlayAnimMontage(ReactionAnimConfig.Montage);
		}
	}

	FVector ReactionDirection = GetReactionDirection(SourcePoint, ImpulseMode);
	ReactionDirection.Z = VerticalLaunchScale;

	if (UCharacterMovementComponent* CharacterMovementComponent = OwnerCharacter->GetCharacterMovement())
	{
		if (AAIController* AIController = Cast<AAIController>(OwnerCharacter->GetController()))
		{
			AIController->StopMovement();
		}

		BeginImpulseMovementOverride();
		CharacterMovementComponent->StopMovementImmediately();
		CharacterMovementComponent->Velocity = ReactionDirection * Magnitude;
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
	if (bImpulseMovementOverrideActive)
	{
		EndImpulseMovementOverride();
	}
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
	ClearImpulseReactionState();
	ActiveCrowdControlTags.Reset();
	RefreshControlBlockTags();
}
