// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MAAttributeSet.h"
#include "Net/UnrealNetwork.h"

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
void UMAAttributeSet::OnRep_##PropertyName(                     \
const FGameplayAttributeData& OldValue)                     \
{                                                               \
GAMEPLAYATTRIBUTE_REPNOTIFY(UMAAttributeSet, PropertyName, OldValue); \
}

void UMAAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Cost, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, MaxCost, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Attack, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
}

DEFINE_REPNOTIFY(Health)
DEFINE_REPNOTIFY(MaxHealth)
DEFINE_REPNOTIFY(Cost)
DEFINE_REPNOTIFY(MaxCost)
DEFINE_REPNOTIFY(Attack)
DEFINE_REPNOTIFY(Armor)
DEFINE_REPNOTIFY(MoveSpeed)
DEFINE_REPNOTIFY(AttackSpeed)

