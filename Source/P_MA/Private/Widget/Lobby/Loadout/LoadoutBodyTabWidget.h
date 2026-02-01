// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadoutBodyTabWidget.generated.h"

class ULoadoutBodyColorPresetData;

UCLASS()
class P_MA_API ULoadoutBodyTabWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout|Body")
	TObjectPtr<ULoadoutBodyColorPresetData> BodyColorPreset;
};
