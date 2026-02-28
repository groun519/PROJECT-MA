// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InBattleStageWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

UCLASS()
class P_MA_API UInBattleStageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="InBattle")
	void SetStageText(const FText& InText);

	UFUNCTION(BlueprintCallable, Category="InBattle")
	void PlayShowAnimation();

	UFUNCTION(BlueprintCallable, Category="InBattle")
	float GetShowAnimationDuration() const;

protected:
	UPROPERTY(meta=(BindWidget))
	UTextBlock* StageText;

	UPROPERTY(Transient, meta=(BindWidgetAnim))
	UWidgetAnimation* ShowAnimation;
};
