// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReadyCheckWidget.generated.h"

class UImage;

UCLASS()
class P_MA_API UReadyCheckWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Ready")
	void SetReadyState(bool bReady);

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ReadyOnImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ReadyOffImage;
};
