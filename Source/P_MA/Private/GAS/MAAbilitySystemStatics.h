// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MAAbilitySystemStatics.generated.h"


class UGameplayAbility;
/**
 * 
 */
UCLASS()
class UMAAbilitySystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static FGameplayTag GetBasicAttackAbilityTag();
	static FGameplayTag GetDeadStatTag();
	static FGameplayTag GetStunStatTag();
	static FGameplayTag GetMovementTag();
	static FGameplayTag GetActiveSkillTag();

	static FGameplayTag GetRushEndTag();
	
	static FGameplayTag GetRotationLockTag();
	static FGameplayTag GetRushingTag();

	static float GetStaticCooldownDurationForAbility(const UGameplayAbility* Ability);
	static float GetStaticCostForAbility(const UGameplayAbility* Ability);
};
