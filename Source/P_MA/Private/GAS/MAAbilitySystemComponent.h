// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MAAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class UMAAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UMAAbilitySystemComponent();
	void ApplyInitialEffects();
	void GiveInitialAbilities();
	void ApplyFullStatEffect();
	
private:
	void AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int Level = 1);
	void HealthUpdated(const FOnAttributeChangeData& ChangeData);
	
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> FullStatEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> DeathEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffects;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TMap<EMAAbilityInputID, TSubclassOf<UGameplayAbility>> Abilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TMap<EMAAbilityInputID, TSubclassOf<UGameplayAbility>> BasicAbilities;
};
