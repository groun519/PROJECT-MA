// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GAS/UtilityModule/UtilityModule.h"
#include "UtilityModule_Composure.generated.h"

/**
 * 
 */
UCLASS()
class UUtilityModule_Composure : public UUtilityModule
{
	GENERATED_BODY()

public:
	UUtilityModule_Composure();
	virtual float ModifyCooldownDuration(float OriginalDuration) const override;
	virtual void ModifyDamageEffectSpec(FGameplayEffectSpecHandle& SpecHandle) const override;
	
private:
	UPROPERTY(EditDefaultsOnly)
	float CooldownMultiplier = 1.5f;

	UPROPERTY(EditDefaultsOnly)
	float DamagePercentAdditive = 0.3f;
};
