// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingScreenWidget.h"
#include "LoadingPlayerStatusWidget.generated.h"

class UImage;

UCLASS(meta = (DisableNativeTick))
class P_MA_API ULoadingPlayerStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetStatus(const FLoadingPlayerStatus& Status);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BackgroundImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BodyColorImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> EyeColorImage;

	UPROPERTY(EditAnywhere, Category = "Loading")
	FLinearColor LoadedBackgroundColor = FLinearColor(0.15f, 0.6f, 0.2f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Loading")
	FLinearColor WaitingBackgroundColor = FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
};
