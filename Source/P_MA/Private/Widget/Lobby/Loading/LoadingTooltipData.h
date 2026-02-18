// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LoadingTooltipData.generated.h"

UCLASS(BlueprintType)
class P_MA_API ULoadingTooltipData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading")
	TArray<FText> Tips;
};
