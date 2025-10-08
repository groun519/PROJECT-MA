// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GameplayAbility_Spin.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/MAAbilitySystemStatics.h"

UGameplayAbility_Spin::UGameplayAbility_Spin()
{
	BlockAbilitiesWithTag.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGameplayAbility_Spin::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayUpperCutMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SkillMontage);
		PlayUpperCutMontageTask->OnBlendOut.AddDynamic(this, &UGameplayAbility_Spin::K2_EndAbility);
		PlayUpperCutMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_Spin::K2_EndAbility);
		PlayUpperCutMontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_Spin::K2_EndAbility);
		PlayUpperCutMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_Spin::K2_EndAbility);
		PlayUpperCutMontageTask->ReadyForActivation();
	}

	if (K2_HasAuthority())
	{
		UAbilityTask_WaitGameplayEvent* WaitTargetEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetSpinDamageTag());
		WaitTargetEventTask->EventReceived.AddDynamic(this, &UGameplayAbility_Spin::DoDamage);
		WaitTargetEventTask->ReadyForActivation();
	}
}

void UGameplayAbility_Spin::DoDamage(FGameplayEventData EventData)
{
	if (K2_HasAuthority())
	{
		TArray<FHitResult> HitResults = GetHitResultFromVirtualSocketTargetData(EventData.TargetData);
		for (FHitResult& HitResult : HitResults)
		{
			ApplyGameplayEffectToHitResultActor(HitResult, SkillDamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}
	}
}

FGameplayTag UGameplayAbility_Spin::GetSpinDamageTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Skill.Spin.Damage");
}
