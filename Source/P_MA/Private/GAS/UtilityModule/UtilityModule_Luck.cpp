// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/UtilityModule/UtilityModule_Luck.h"

float UUtilityModule_Luck::ModifyCooldownDuration(float OriginalDuration) const
{
	float RandNum = FMath::RandRange(1.f,100.f);
	if (RandNum <= ResetP)
	{
		return OriginalDuration * 0.001f;
	}
	return OriginalDuration;
}
