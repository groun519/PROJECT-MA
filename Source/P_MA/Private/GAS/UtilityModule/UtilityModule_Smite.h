// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GAS/UtilityModule/UtilityModule.h"
#include "UtilityModule_Smite.generated.h"

/**
 * 
 */
UCLASS()
class UUtilityModule_Smite : public UUtilityModule
{
	GENERATED_BODY()


public:
	UPROPERTY(EditDefaultsOnly)
	float DamagePercentAdditive = 0.3f;

	virtual void ModifyDamageEffectSpec(FGameplayEffectSpecHandle& SpecHandle) const override;

private:
	//MMC에서 읽어올 태그 (Data.Damage.UtilityMultiplier)
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag DamageModifierTag;
};
