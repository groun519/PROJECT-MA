// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MAGameplayAbilityTypes.generated.h"

UENUM(BlueprintType)
enum class EMAAbilityInputID : uint8
{
	None				UMETA(DisplayName = "None"),

	BasicAttack			UMETA(DisplayName = "Basic Attack"),
	SubAbility			UMETA(DisplayName = "Ability Passive"),

	Confirm				UMETA(DisplayName = "Confirm"),
	Cancel				UMETA(DisplayName = "Cancel"),
};