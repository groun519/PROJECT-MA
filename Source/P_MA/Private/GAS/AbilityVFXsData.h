// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AbilityVFXsData.generated.h"

class UNiagaraSystem;
/**
 * 
 */
UCLASS()
class UAbilityVFXsData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, TObjectPtr<UNiagaraSystem>> EffectMap;
};
