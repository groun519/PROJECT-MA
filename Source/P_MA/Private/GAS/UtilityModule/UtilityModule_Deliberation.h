// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GAS/UtilityModule/UtilityModule.h"
#include "UtilityModule_Deliberation.generated.h"

/**
 * 데미지 증가 + 공격 속도 감소 모듈
 * 데미지 증가 : 합연산
*/
UCLASS()
class UUtilityModule_Deliberation : public UUtilityModule
{
	GENERATED_BODY()

public:
	UUtilityModule_Deliberation();
	virtual void ModifyDamageEffectSpec(FGameplayEffectSpecHandle& SpecHandle) const override;
	virtual float ModifyMontagePlayRate(float OriginalPlayRate) const override;

private:
	UPROPERTY(EditDefaultsOnly)
	float DamagePercentAdditive = 0.3f;

	UPROPERTY(EditDefaultsOnly)
	float MontagePlayRate = 0.85f;

	FGameplayTag DamageModifierTag;
};
