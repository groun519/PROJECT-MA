// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LoadingBackgroundData.generated.h"

class UTexture2D;

UCLASS(BlueprintType)
class P_MA_API ULoadingBackgroundData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading")
	TArray<TObjectPtr<UTexture2D>> BackgroundImages;
};
