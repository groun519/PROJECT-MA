// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GameplayAbility_Jump.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/MAAbilitySystemStatics.h"

UGameplayAbility_Jump::UGameplayAbility_Jump()
{
	BlockAbilitiesWithTag.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGameplayAbility_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                            const FGameplayEventData* TriggerEventData)
{
	if (!SkillMontage || !K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	// 어빌리티 태스크를 사용하여 몽타주를 재생합니다.
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SkillMontage);
	
	// 몽타주가 끝나거나, 취소되거나, 중단되면 어빌리티를 종료합니다.
	PlayMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_Jump::K2_EndAbility);
	PlayMontageTask->OnBlendOut.AddDynamic(this, &UGameplayAbility_Jump::K2_EndAbility);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_Jump::K2_EndAbility);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_Jump::K2_EndAbility);
	
	PlayMontageTask->ReadyForActivation();
}
