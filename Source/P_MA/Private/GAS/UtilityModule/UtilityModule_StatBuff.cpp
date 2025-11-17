// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/UtilityModule/UtilityModule_StatBuff.h"

#include "GameplayEffect.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"

void UUtilityModule_StatBuff::OnAbilityEnd_Implementation(bool bWasCancelled)
{
	if (bWasCancelled || !StatBuffGE || !OwningAbility || !OwningAbility->K2_HasAuthority())
		return;

	UAbilitySystemComponent* ASC = OwningAbility->GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		FGameplayEffectSpecHandle SpecHandle = OwningAbility->MakeOutgoingGameplayEffectSpec(StatBuffGE, OwningAbility->GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			OwningAbility->ApplyGESpecToOwner(SpecHandle);
		}
	}
}
