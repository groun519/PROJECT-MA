// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MAAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class UMAAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const override;
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, Health)
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, MaxHealth)
	// ATTRIBUTE_ACCESSORS(UMAAttributeSet, Cost)
	// ATTRIBUTE_ACCESSORS(UMAAttributeSet, MaxCost)
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, Attack)
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, Armor)
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, MoveSpeed)
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, AttackSpeed)


private:
	UPROPERTY(ReplicatedUsing = OnRep_Health)			FGameplayAttributeData Health;
	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth)		FGameplayAttributeData MaxHealth;
	// UPROPERTY(ReplicatedUsing = OnRep_Cost)			FGameplayAttributeData Cost;
	// UPROPERTY(ReplicatedUsing = OnRep_MaxCost)		FGameplayAttributeData MaxCost;
	UPROPERTY(ReplicatedUsing = OnRep_Attack)			FGameplayAttributeData Attack;
	UPROPERTY(ReplicatedUsing = OnRep_Armor)			FGameplayAttributeData Armor;
	UPROPERTY(ReplicatedUsing = OnRep_MoveSpeed)		FGameplayAttributeData MoveSpeed;
	UPROPERTY(ReplicatedUsing = OnRep_AttackSpeed)		FGameplayAttributeData AttackSpeed;
	
	UFUNCTION()	void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	// UFUNCTION() void OnRep_Cost(const FGameplayAttributeData& OldValue);
	// UFUNCTION() void OnRep_MaxCost(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Attack(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Armor(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_AttackSpeed(const FGameplayAttributeData& OldValue);
	
};
