// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GameplayAbility_Stab.h"
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
		UAbilityTask_PlayMontageAndWait* PlayStabMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, StabMontage);
		PlayStabMontageTask -> OnBlendOut.AddDynamic(this, &UGameplayAbility_Stab::K2_EndAbility);
		PlayStabMontageTask -> OnCancelled.AddDynamic(this, &UGameplayAbility_Stab::K2_EndAbility);
		PlayStabMontageTask -> OnInterrupted.AddDynamic(this, &UGameplayAbility_Stab::K2_EndAbility);
		PlayStabMontageTask -> OnCompleted.AddDynamic(this, &UGameplayAbility_Stab::K2_EndAbility);
		PlayStabMontageTask -> ReadyForActivation();
	}
}
