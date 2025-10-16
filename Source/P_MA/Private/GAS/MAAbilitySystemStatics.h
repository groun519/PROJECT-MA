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
	
	static FGameplayTag GetRotationLockTag();
	static FGameplayTag GetRushingTag();
	static FGameplayTag GetAimingTag();

	static FGameplayTag GetHealthFullStatTag();
	static FGameplayTag GetHealthEmptyStatTag();

	static FGameplayTag GetPlayerRoleTag();
	static FGameplayTag GetGoldAttributeTag();

	static FGameplayTag GetMontageDamageTag();
	static FGameplayTag GetSkillAttributeTag();
	static FGameplayTag GetChargeSkillTag();
	static FGameplayTag GetHoldSkillTag();
	static FGameplayTag GetChainSkillTag();

	static bool IsPlayer(const AActor* ActorToCheck);

	static float GetStaticCooldownDurationForAbility(const UGameplayAbility* Ability);
	static float GetStaticCostForAbility(const UGameplayAbility* Ability);
};
