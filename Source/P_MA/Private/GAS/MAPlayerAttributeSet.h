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
	ATTRIBUTE_ACCESSORS(UMAPlayerAttributeSet, Gold)
	ATTRIBUTE_ACCESSORS(UMAPlayerAttributeSet, AttackRange)

private:
	UPROPERTY(ReplicatedUsing = OnRep_Gold)				FGameplayAttributeData Gold;
	UPROPERTY(ReplicatedUsing = OnRep_AttackRange)		FGameplayAttributeData AttackRange;
	
	UFUNCTION() void OnRep_Gold(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_AttackRange(const FGameplayAttributeData& OldValue);

};
