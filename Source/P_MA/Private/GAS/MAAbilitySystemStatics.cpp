// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MAAbilitySystemStatics.h"

FGameplayTag UMAAbilitySystemStatics::GetBasicAttackAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.BasicAttack");
}

FGameplayTag UMAAbilitySystemStatics::GetDeadStatTag()
{
	return FGameplayTag::RequestGameplayTag("Stats.Dead");
}

FGameplayTag UMAAbilitySystemStatics::GetStunStatTag()
{
	return FGameplayTag::RequestGameplayTag("Stats.Stun");
}

FGameplayTag UMAAbilitySystemStatics::GetMovementTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Movement");
}

FGameplayTag UMAAbilitySystemStatics::GetActiveSkillTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Skill");
}
