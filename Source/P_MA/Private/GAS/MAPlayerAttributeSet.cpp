// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MAPlayerAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

/*
* void UNVAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
* {
* 	GAMEPLAYATTRIBUTE_REPNOTIFY(UNVAttributeSet, Health, OldValue)
* }
*
* ^ ^ ^ ^ ^ ^ ^ ^ ^ ^
*
* DEFINE_REPNOTIFY(Health)
*/
#define DEFINE_REPNOTIFY(PropertyName)                              \
void UMAPlayerAttributeSet::OnRep_##PropertyName(                     \
const FGameplayAttributeData& OldValue)                     \
{                                                               \
GAMEPLAYATTRIBUTE_REPNOTIFY(UMAPlayerAttributeSet, PropertyName, OldValue); \
}

void UMAPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UMAPlayerAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAPlayerAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAPlayerAttributeSet, Attack, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAPlayerAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAPlayerAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAPlayerAttributeSet, Gold, COND_None, REPNOTIFY_Always);
}

DEFINE_REPNOTIFY(Health)
DEFINE_REPNOTIFY(MaxHealth)
DEFINE_REPNOTIFY(Attack)
DEFINE_REPNOTIFY(MoveSpeed)
DEFINE_REPNOTIFY(AttackSpeed)
DEFINE_REPNOTIFY(Gold)

