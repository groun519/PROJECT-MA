// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/UtilityModule/UtilityModule.h"
#include "UtilityModule_Luck.generated.h"

/**
 * 
 */
UCLASS()
class UUtilityModule_Luck : public UUtilityModule
{
	GENERATED_BODY()

public:
	virtual float ModifyCooldownDuration(float OriginalDuration) const override;

private:
	UPROPERTY(EditDefaultsOnly)
	float ResetP = 40.f;
};
