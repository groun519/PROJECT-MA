// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GameplayAbility_Stab.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/MAAbilitySystemStatics.h"

UGameplayAbility_Stab::UGameplayAbility_Stab()
{
	BlockAbilitiesWithTag.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGameplayAbility_Stab::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
		UAbilityTask_PlayMontageAndWait* PlayStabMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SkillMontage);
		PlayStabMontageTask -> OnBlendOut.AddDynamic(this, &UGameplayAbility_Stab::K2_EndAbility);
		PlayStabMontageTask -> OnCancelled.AddDynamic(this, &UGameplayAbility_Stab::K2_EndAbility);
		PlayStabMontageTask -> OnInterrupted.AddDynamic(this, &UGameplayAbility_Stab::K2_EndAbility);
		PlayStabMontageTask -> OnCompleted.AddDynamic(this, &UGameplayAbility_Stab::K2_EndAbility);
		PlayStabMontageTask -> ReadyForActivation();
	}

	if (K2_HasAuthority())
	{
		UAbilityTask_WaitGameplayEvent* WaitTargetEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetStabDamageTag());
		WaitTargetEventTask->EventReceived.AddDynamic(this, &UGameplayAbility_Stab::DoDamage);
		WaitTargetEventTask->ReadyForActivation();
	}
}

void UGameplayAbility_Stab::DoDamage(FGameplayEventData EventData)
{
	if (K2_HasAuthority())
	{
		TArray<FHitResult> HitResults = GetHitResultFromVirtualSocketTargetData(EventData.TargetData, ETeamAttitude::Hostile, ShouldDrawDebug(), true);
		for (FHitResult& HitResult : HitResults)
		{
			ApplyGameplayEffectToHitResultActor(HitResult, SkillDamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}
	}
}

FGameplayTag UGameplayAbility_Stab::GetStabDamageTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Skill.Stab.Damage");
}
