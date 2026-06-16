#include "Character/MAImpulseComponent.h"

#include "Character/MACharacter.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "P_MA/P_MA.h"

UMAImpulseComponent::UMAImpulseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMAImpulseComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<AMACharacter>(GetOwner());
}

AMACharacter* UMAImpulseComponent::ResolveOwnerCharacter()
{
	if (!OwnerCharacter) OwnerCharacter = Cast<AMACharacter>(GetOwner());
	return OwnerCharacter;
}

UCharacterMovementComponent* UMAImpulseComponent::GetOwnerCharacterMovement() const
{
	return OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr;
}

UCapsuleComponent* UMAImpulseComponent::GetOwnerCapsule() const
{
	return OwnerCharacter ? OwnerCharacter->GetCapsuleComponent() : nullptr;
}

void UMAImpulseComponent::ApplyStatusEffectImpulse(EStatusEffectImpulseMode ImpulseMode, float Magnitude, const FVector& SourcePoint, const FGameplayTag& StatusEffectTag)
{
	AMACharacter* Character = ResolveOwnerCharacter();
	if (!Character || Character->IsDead() || Magnitude <= 0.f) return;

	FStatusEffectAnimConfig StatusEffectAnimConfig;
	float VerticalLaunchScale = 0.f;
	if (Character->GetStatusEffectAnimConfig(StatusEffectTag, StatusEffectAnimConfig))
	{
		VerticalLaunchScale = StatusEffectAnimConfig.VerticalLaunchScale;
	}

	const FVector OwnerLocation = OwnerCharacter->GetActorLocation();
	FVector StatusEffectDirection = FVector::ZeroVector;
	switch (ImpulseMode)
	{
	case EStatusEffectImpulseMode::PushFromSource:
		StatusEffectDirection = (OwnerLocation - SourcePoint).GetSafeNormal2D();
		break;
	case EStatusEffectImpulseMode::PullToSource:
		StatusEffectDirection = (SourcePoint - OwnerLocation).GetSafeNormal2D();
		break;
	default:
		break;
	}
	if (!FMath::IsNearlyZero(VerticalLaunchScale)) StatusEffectDirection.Z = VerticalLaunchScale;

	CancelInterruptibleActionImpulses();
	ApplyImpulseVelocityInternal(StatusEffectTag, StatusEffectDirection * Magnitude, !bImpulseMovementOverrideActive);
}

FMAActionImpulseHandle UMAImpulseComponent::ApplyActionImpulseVelocity(
	UObject* OwnerObject,
	const FGameplayTag& ImpulseTag,
	const FVector& Velocity,
	float Duration,
	bool bStopMovementImmediately)
{
	AMACharacter* Character = ResolveOwnerCharacter();
	if (!Character || Character->IsDead()) return {};
	// TODO: Add an explicit policy flag for movement-lock-immune hard movement skills.
	if (Character->IsMovementBlocked()) return {};
	if (!OwnerObject) return {};
	if (!ImpulseTag.IsValid() || Velocity.IsNearlyZero() || Duration <= 0.f) return {};
	CancelInterruptibleActionImpulses();
	ApplyImpulseVelocityInternal(ImpulseTag, Velocity, bStopMovementImmediately);

	if (++NextActionImpulseInstanceId == 0) ++NextActionImpulseInstanceId;
	const FMAActionImpulseHandle Handle{ ImpulseTag, NextActionImpulseInstanceId };
	ActiveActionImpulseOwners.FindOrAdd(ImpulseTag) = FActionImpulseOwner{ OwnerObject, Handle.InstanceId };
	ScheduleActionImpulseStop(ImpulseTag, Duration);

	if (UMASkillAbility* SkillAbility = Cast<UMASkillAbility>(OwnerObject))
	{
		FMASkillEvent Event(UMAAbilitySystemStatics::GetMovementStartEventTag());
		Event.Payloads.SetStruct(UMAAbilitySystemStatics::GetMovementHandleTag(), Handle);
		UMASkillEventRoutingStatics::TryNotifySkillEvent(
			SkillAbility,
			MoveTemp(Event),
			SkillAbility->GetCurrentBindingScope());
	}

	return Handle;
}

bool UMAImpulseComponent::IsActionImpulseActive(const FMAActionImpulseHandle& Handle) const
{
	const FActionImpulseOwner* ActionImpulseOwner = ActiveActionImpulseOwners.Find(Handle.Tag);
	return Handle.IsValid() && ActionImpulseOwner && ActionImpulseOwner->InstanceId == Handle.InstanceId;
}

void UMAImpulseComponent::RemoveImpulse(const FGameplayTag& StatusEffectTag)
{
	if (ActiveImpulseContributions.Remove(StatusEffectTag) == 0) return;

	ActiveActionImpulseOwners.Remove(StatusEffectTag);
	ClearActionImpulseTimer(StatusEffectTag);

	if (ActiveImpulseContributions.IsEmpty())
	{
		ClearImpulseState();
		return;
	}

	RecalculateImpulseVelocity(false);
}

void UMAImpulseComponent::StopOwnedActionImpulses(UObject* OwnerObject)
{
	if (!OwnerObject || ActiveActionImpulseOwners.IsEmpty()) return;

	TArray<FGameplayTag> OwnedImpulseTags;
	for (const TPair<FGameplayTag, FActionImpulseOwner>& OwnerPair : ActiveActionImpulseOwners)
	{
		if (OwnerPair.Value.Object.Get() != OwnerObject) continue;
		OwnedImpulseTags.Add(OwnerPair.Key);
	}

	for (const FGameplayTag& OwnedImpulseTag : OwnedImpulseTags)
	{
		RemoveImpulse(OwnedImpulseTag);
	}
}

void UMAImpulseComponent::CancelInterruptibleActionImpulses()
{
	if (ActiveActionImpulseOwners.IsEmpty()) return;

	TArray<FGameplayTag> ActionImpulseTags;
	ActiveActionImpulseOwners.GetKeys(ActionImpulseTags);
	for (const FGameplayTag& ActionImpulseTag : ActionImpulseTags)
	{
		RemoveImpulse(ActionImpulseTag);
	}
}

void UMAImpulseComponent::ResetImpulseState()
{
	ClearImpulseState();
}

void UMAImpulseComponent::ApplyImpulseVelocityInternal(const FGameplayTag& ImpulseTag, const FVector& Velocity, bool bStopMovementImmediately)
{
	if (!GetOwnerCharacterMovement()) return;

	BeginImpulseMovementOverride();
	ActiveImpulseContributions.Add(ImpulseTag, Velocity);
	RecalculateImpulseVelocity(bStopMovementImmediately);
}

void UMAImpulseComponent::ScheduleActionImpulseStop(const FGameplayTag& ImpulseTag, float Duration)
{
	UWorld* World = GetWorld();
	if (!World || !ImpulseTag.IsValid() || Duration <= 0.f) return;

	ClearActionImpulseTimer(ImpulseTag);

	FTimerHandle& TimerHandle = ActiveActionImpulseTimers.FindOrAdd(ImpulseTag);
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateWeakLambda(this, [this, ImpulseTag]()
	{
		RemoveImpulse(ImpulseTag);
	});
	World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Duration, false);
}

void UMAImpulseComponent::ClearActionImpulseTimer(const FGameplayTag& ImpulseTag)
{
	FTimerHandle* TimerHandle = ActiveActionImpulseTimers.Find(ImpulseTag);
	if (!TimerHandle) return;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(*TimerHandle);
	}

	ActiveActionImpulseTimers.Remove(ImpulseTag);
}

void UMAImpulseComponent::ClearAllActionImpulseTimers()
{
	if (ActiveActionImpulseTimers.IsEmpty()) return;

	if (UWorld* World = GetWorld())
	{
		for (TPair<FGameplayTag, FTimerHandle>& TimerPair : ActiveActionImpulseTimers)
		{
			World->GetTimerManager().ClearTimer(TimerPair.Value);
		}
	}

	ActiveActionImpulseTimers.Reset();
}

void UMAImpulseComponent::BeginImpulseMovementOverride()
{
	AMACharacter* Character = ResolveOwnerCharacter();
	if (!Character || Character->IsDead()) return;

	UCharacterMovementComponent* Move = GetOwnerCharacterMovement();
	UCapsuleComponent* Capsule = GetOwnerCapsule();
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

void UMAImpulseComponent::EndImpulseMovementOverride()
{
	if (!ResolveOwnerCharacter() || !bImpulseMovementOverrideActive) return;

	if (UCharacterMovementComponent* Move = GetOwnerCharacterMovement())
	{
		Move->GroundFriction = SavedImpulseGroundFriction;
		Move->BrakingFrictionFactor = SavedImpulseBrakingFrictionFactor;
		Move->BrakingDecelerationWalking = SavedImpulseBrakingDecelerationWalking;
	}

	if (UCapsuleComponent* Capsule = GetOwnerCapsule())
	{
		Capsule->SetCollisionResponseToChannel(ECC_Hitbox, SavedImpulseHitboxResponse);
	}

	bImpulseMovementOverrideActive = false;
}

void UMAImpulseComponent::ClearImpulseState()
{
	ActiveImpulseContributions.Reset();
	ActiveActionImpulseOwners.Reset();
	ClearAllActionImpulseTimers();
	if (UCharacterMovementComponent* CharacterMovementComponent = GetOwnerCharacterMovement())
	{
		// TODO: Revisit this if we add falling, momentum-preserving movement, or any motion
		// that should survive after the final impulse ends. The current movement model treats
		// impulse motion as fully authoritative, so clearing the last impulse also zeros velocity.
		CharacterMovementComponent->Velocity = FVector::ZeroVector;
	}

	if (!bImpulseMovementOverrideActive) return;
	EndImpulseMovementOverride();
}

void UMAImpulseComponent::RecalculateImpulseVelocity(bool bStopMovementImmediately)
{
	UCharacterMovementComponent* CharacterMovementComponent = GetOwnerCharacterMovement();
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
