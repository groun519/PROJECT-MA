// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MAAbilitySystemStatics.generated.h"


class UGameplayAbility;
struct FGameplayAbilitySpec;
class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class UMAAbilitySystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static FGameplayTag GetBasicAttackAbilityTag();
	static FGameplayTag GetIgnoreClearTag();
	static FGameplayTag GetDeadStatTag();
	static FGameplayTag GetStunStatTag();
	
	static FGameplayTag GetRotationLockTag();
	static FGameplayTag GetRushingTag();
	static FGameplayTag GetAimingTag();
	static FGameplayTag GetChargingTag();

	static FGameplayTag GetHealthFullStatTag();
	static FGameplayTag GetHealthEmptyStatTag();

	static FGameplayTag GetPlayerRoleTag();
	static FGameplayTag GetGoldAttributeTag();

	static FGameplayTag GetMontageDamageTag();
	static FGameplayTag GetLaunchActivateTag();

	static FGameplayTag GetBehaviorMultiplierTag();
	static FGameplayTag GetElementalMultiplierTag();
	static FGameplayTag GetUtilityMultiplierTag();

	static FGameplayTag GetAirborneTag();
	static FGameplayTag GetKnockdownTag();
	static FGameplayTag GetRecoveryTag();
	
	static bool IsPlayer(const AActor* ActorToCheck);

	static float GetStaticCooldownDurationForAbility(const UGameplayAbility* Ability);
	static float GetStaticCostForAbility(const UGameplayAbility* Ability);

	static bool CheckAbilityCost(const FGameplayAbilitySpec& AbilitySpec, const UAbilitySystemComponent& ASC);
	static bool CheckAbilityCostStatic(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC);
	static float GetCooldownDurationFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC, int AbilityLevel);
	static float GetCooldownRemainingFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC);
};
