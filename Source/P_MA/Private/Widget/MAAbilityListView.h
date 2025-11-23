// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ListView.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MAAbilityListView.generated.h"

class UGameplayAbility;

/**
 * 
 * 
 */
UCLASS()
class UMAAbilitySlotDataObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	EMAAbilityInputID InputID;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	TSubclassOf<UGameplayAbility> AbilityClass;
};

/**
 * 
 */
UCLASS()
class UMAAbilityListView : public UListView
{
	GENERATED_BODY()

public:
	void ConfigureAbilities(const TMap<EMAAbilityInputID, TSubclassOf<UGameplayAbility>>& Abilities);
	
};