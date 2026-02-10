// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoopPlayerStatusWidget.generated.h"

class UImage;

USTRUCT(BlueprintType)
struct FLoopReadyPlayerStatus
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LoopReady")
	bool bReady = false;

	UPROPERTY(BlueprintReadOnly, Category = "LoopReady")
	FLinearColor BodyColor = FLinearColor::Black;

	UPROPERTY(BlueprintReadOnly, Category = "LoopReady")
	FLinearColor EyeColor = FLinearColor::White;
};

UCLASS()
class P_MA_API ULoopPlayerStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetStatus(const FLoopReadyPlayerStatus& Status);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BackgroundImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BodyColorImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> EyeColorImage;

	UPROPERTY(EditAnywhere, Category = "LoopReady")
	FLinearColor ReadyBackgroundColor = FLinearColor(0.15f, 0.6f, 0.2f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "LoopReady")
	FLinearColor WaitingBackgroundColor = FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
};
