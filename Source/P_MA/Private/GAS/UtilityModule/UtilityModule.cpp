// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/UtilityModule/UtilityModule.h"

#include "GAS/MAAbilitySystemStatics.h"

UUtilityModule::UUtilityModule()
{
	DamageModifierTag=UMAAbilitySystemStatics::GetUtilityMultiplierTag();
}

void UUtilityModule::OnAbilityActivate_Implementation()
{
}

void UUtilityModule::OnAbilityEnd_Implementation(bool bWasCancelled)
{
}
