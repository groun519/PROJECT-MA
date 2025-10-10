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
	void InitializeBaseAttributes();
	void ServerSideInit();
	void ApplyFullStatEffect();
	const TMap<EMAAbilityInputID, TSubclassOf<UGameplayAbility>>& GetAbilities() const;
	
private:
	void ApplyInitialEffects();
	void GiveInitialAbilities();
	void AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int Level = 1);
	void HealthUpdated(const FOnAttributeChangeData& ChangeData);

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TMap<EMAAbilityInputID, TSubclassOf<UGameplayAbility>> Abilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TMap<EMAAbilityInputID, TSubclassOf<UGameplayAbility>> BasicAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	class UPA_AbilitySystemGenerics* AbilitySystemGenerics;
};
