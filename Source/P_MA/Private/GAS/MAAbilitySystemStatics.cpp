// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MAAbilitySystemStatics.h"

FGameplayTag UMAAbilitySystemStatics::GetBasicAttackAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack");
}
