// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MAAbilitySystemGlobals.h"

#include "MAGameplayAbilityTypes.h"

FGameplayEffectContext* UMAAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FMAGameplayEffectContext();
}
