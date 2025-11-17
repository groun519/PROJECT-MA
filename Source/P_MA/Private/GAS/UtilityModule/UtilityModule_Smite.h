// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GAS/UtilityModule/UtilityModule.h"
#include "UtilityModule_Smite.generated.h"

/**
 * 강타 모듈: 스킬의 최종 데미지를 증가시킴
 * 플레이어 공격력 스탯 기반으로 n% 증가
 */
UCLASS()
class UUtilityModule_Smite : public UUtilityModule
{
	GENERATED_BODY()


public:
	UUtilityModule_Smite();
	
	UPROPERTY(EditDefaultsOnly)
	float DamagePercentAdditive = 0.3f;

	virtual void ModifyDamageEffectSpec(FGameplayEffectSpecHandle& SpecHandle) const override;

private:
	FGameplayTag DamageModifierTag;
};
