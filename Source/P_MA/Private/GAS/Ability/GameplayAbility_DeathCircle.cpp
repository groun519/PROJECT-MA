// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GameplayAbility_DeathCircle.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/MAAbilitySystemStatics.h"

UGameplayAbility_DeathCircle::UGameplayAbility_DeathCircle()
{
	BlockAbilitiesWithTag.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
	
}

void UGameplayAbility_DeathCircle::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SkillMontage);
		PlayMontageTask->OnBlendOut.AddDynamic(this, &UGameplayAbility_DeathCircle::K2_EndAbility);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_DeathCircle::K2_EndAbility);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_DeathCircle::K2_EndAbility);
		PlayMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_DeathCircle::K2_EndAbility);
		PlayMontageTask->ReadyForActivation();
	}
}
