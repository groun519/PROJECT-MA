// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/UtilityModule/UtilityModule.h"
#include "UtilityModule_Haste.generated.h"

/**
 * 스킬의 기본 쿨타임 감소 모듈
 * 기본 쿨타임 * 0.n
 */
UCLASS()
class UUtilityModule_Haste : public UUtilityModule
{
	GENERATED_BODY()

public:
	virtual float ModifyCooldownDuration(float OriginalDuration) const override;

private:
	UPROPERTY(EditDefaultsOnly)
	float CooldownMultiplier = 0.8f;
};
