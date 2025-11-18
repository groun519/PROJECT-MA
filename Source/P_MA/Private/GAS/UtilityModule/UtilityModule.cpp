// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/UtilityModule/UtilityModule.h"

#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"

UUtilityModule::UUtilityModule()
{
	DamageModifierTag=UMAAbilitySystemStatics::GetUtilityMultiplierTag();
}

void UUtilityModule::OnAbilityActivate_Implementation()
{
	if (!OwningAbility || !OwningAbility->K2_HasAuthority())
		return;
	
	UAbilitySystemComponent* ASC = OwningAbility->GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		FGameplayEffectSpecHandle SpecHandle = OwningAbility->MakeOutgoingGameplayEffectSpec(BuffGEOnActive, OwningAbility->GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			OwningAbility->ApplyGESpecToOwner(SpecHandle);
		}
	}
}

void UUtilityModule::OnAbilityEnd_Implementation(bool bWasCancelled)
{
	if (bWasCancelled || !BuffGEOnEnd || !OwningAbility || !OwningAbility->K2_HasAuthority())
		return;
	UAbilitySystemComponent* ASC = OwningAbility->GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		FGameplayEffectSpecHandle SpecHandle = OwningAbility->MakeOutgoingGameplayEffectSpec(BuffGEOnEnd, OwningAbility->GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			OwningAbility->ApplyGESpecToOwner(SpecHandle);
		}
	}
}

void UUtilityModule::ModifyDamageEffectSpec(FGameplayEffectSpecHandle& SpecHandle) const
{
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(DamageModifierTag,DamagePercentAdditive);
	}
}

float UUtilityModule::ModifyCooldownDuration(float OriginalDuration) const
{
	if (CooldownMultiplier == 0.f)
	{
		float RandNum = FMath::RandRange(1.f, 100.f);
		if (RandNum <= ChanceToReset)
		{
			return 0.f;
		}
		else
		{
			return OriginalDuration;
		}
	}
	return OriginalDuration * CooldownMultiplier;
}

float UUtilityModule::ModifyMontagePlayRate(float OriginalPlayRate) const
{
	return OriginalPlayRate *MontagePlayRate;
}

void UUtilityModule::InitFromData(const FSkillUtilityModule& Data)
{
	DamagePercentAdditive = Data.DamagePercentAdditive;
	MontagePlayRate = Data.MontagePlayRate;
	CooldownMultiplier = Data.CooldownMultiplier;
	ChanceToReset = Data.ChanceToReset;
	BuffGEOnActive = Data.BuffGEOnActive;
	BuffGEOnEnd = Data.BuffGEOnEnd;
}
