// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MAGameplayAbilityTypes.generated.h"

UENUM(BlueprintType)
enum class EMAAbilityInputID : uint8
{
	None				UMETA(DisplayName = "None"),

	Attack				UMETA(DisplayName = "Attack"),
	Skill				UMETA(DisplayName = "Skill"),

	Confirm				UMETA(DisplayName = "Confirm"),
	Cancel				UMETA(DisplayName = "Cancel"),
};