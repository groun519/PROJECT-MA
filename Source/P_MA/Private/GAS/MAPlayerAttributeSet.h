// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MAPlayerAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class UMAPlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	
	ATTRIBUTE_ACCESSORS(UMAPlayerAttributeSet, Health)
	ATTRIBUTE_ACCESSORS(UMAPlayerAttributeSet, MaxHealth)
	ATTRIBUTE_ACCESSORS(UMAPlayerAttributeSet, Attack)
	ATTRIBUTE_ACCESSORS(UMAPlayerAttributeSet, MoveSpeed)
	ATTRIBUTE_ACCESSORS(UMAPlayerAttributeSet, AttackSpeed)
	ATTRIBUTE_ACCESSORS(UMAPlayerAttributeSet, Gold)


private:
	UPROPERTY(ReplicatedUsing = OnRep_Health)			FGameplayAttributeData Health;
	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth)		FGameplayAttributeData MaxHealth;
	UPROPERTY(ReplicatedUsing = OnRep_Attack)			FGameplayAttributeData Attack;
	UPROPERTY(ReplicatedUsing = OnRep_MoveSpeed)		FGameplayAttributeData MoveSpeed;
	UPROPERTY(ReplicatedUsing = OnRep_AttackSpeed)		FGameplayAttributeData AttackSpeed;
	UPROPERTY(ReplicatedUsing = OnRep_Gold)				FGameplayAttributeData Gold;
	
	UFUNCTION()	void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Attack(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_AttackSpeed(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Gold(const FGameplayAttributeData& OldValue);

};
