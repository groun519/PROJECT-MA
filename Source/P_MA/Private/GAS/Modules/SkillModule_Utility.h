// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Modules/MASkillModule.h"
#include "SkillModule_Utility.generated.h"

/**
 * 
 */
UCLASS()
class USkillModule_Utility : public UMASkillModule
{
	GENERATED_BODY()

public:
	virtual void ModifyDamageSpec(FGameplayEffectSpecHandle& SpecHandle) const override;
	virtual void ModifyCooldownSpec(FGameplayEffectSpecHandle& SpecHandle) const override;
	virtual float GetAnimSpeedMultiplier() const override {return AnimSpeedMultiplier;}
	
protected:
	UPROPERTY()
	float DamageMultiplier = 1.f;
	UPROPERTY()
	FGameplayTag DamageModTag = FGameplayTag::RequestGameplayTag("Data.Damage.UtilityModifier");
	UPROPERTY()
	float CooldownMultiplier = 1.f;
	UPROPERTY()
	FGameplayTag CooldownModTag = FGameplayTag::RequestGameplayTag("Data.Cooldown.Duration");

	UPROPERTY()
	float AnimSpeedMultiplier=1.f;
};
