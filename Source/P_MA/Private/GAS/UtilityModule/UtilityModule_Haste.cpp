// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/UtilityModule/UtilityModule_Haste.h"

float UUtilityModule_Haste::ModifyCooldownDuration(float OriginalDuration) const
{
	return OriginalDuration * CooldownMultiplier;
}
