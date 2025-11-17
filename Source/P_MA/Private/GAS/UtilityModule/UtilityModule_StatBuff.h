// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/UtilityModule/UtilityModule.h"
#include "UtilityModule_StatBuff.generated.h"

/**
 * 캐릭터 스탯 일시적으로 올려주는 버프형 모듈
 */
UCLASS()
class UUtilityModule_StatBuff : public UUtilityModule
{
	GENERATED_BODY()

public:
	virtual void OnAbilityEnd_Implementation(bool bWasCancelled) override;

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UGameplayEffect> StatBuffGE;
};
