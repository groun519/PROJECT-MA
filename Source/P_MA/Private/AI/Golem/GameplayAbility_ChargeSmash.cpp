// GameplayAbility_ChargeSmash.cpp

#include "AI/Golem/GameplayAbility_ChargeSmash.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/MAAttributeSet.h"

bool UGameplayAbility_ChargeSmash::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		float Fury = ASC->GetNumericAttribute(UMAAttributeSet::GetFuryAttribute());
		if (Fury < 15.f)
		{
			return false;
		}
	}

	return true;
}

void UGameplayAbility_ChargeSmash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		float Fury = ASC->GetNumericAttribute(UMAAttributeSet::GetFuryAttribute());
		if (Fury < 15.f)
		{
			K2_EndAbility();
			return;
		}
	}

	if (!K2_CommitAbility()) 
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!ChargeSmashMontage)
		{
			K2_EndAbility();
			return;
		}

		UAbilityTask_PlayMontageAndWait* PlayChargeSmashMontageTask = 
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ChargeSmashMontage);

		PlayChargeSmashMontageTask->OnBlendOut.AddDynamic(this, &UGameplayAbility_ChargeSmash::K2_EndAbility);
		PlayChargeSmashMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_ChargeSmash::K2_EndAbility);
		PlayChargeSmashMontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_ChargeSmash::K2_EndAbility);
		PlayChargeSmashMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_ChargeSmash::K2_EndAbility);

		PlayChargeSmashMontageTask->ReadyForActivation();
	}

	UAbilityTask_WaitGameplayEvent* WaitLaunchEventTask = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetChargeSmashTag());

	if (WaitLaunchEventTask)
	{
		WaitLaunchEventTask->EventReceived.AddDynamic(this, &UGameplayAbility_ChargeSmash::StartCharging);
		WaitLaunchEventTask->ReadyForActivation();
	}
}

FGameplayTag UGameplayAbility_ChargeSmash::GetChargeSmashTag() const
{
	return FGameplayTag::RequestGameplayTag("Ability.Monster.ChargeSmash");
}

void UGameplayAbility_ChargeSmash::StartCharging(FGameplayEventData EventData)
{
	if (K2_HasAuthority())
	{
		TArray<FHitResult> HitTargets =
			GetHitResultFromVirtualSocketTargetData(EventData.TargetData, ETeamAttitude::Hostile, false, true);

		for (auto& Hit : HitTargets)
		{
			if (AActor* HitActor = Hit.GetActor())
			{
			}
		}
	}
}
