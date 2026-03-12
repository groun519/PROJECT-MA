// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MADamageTextWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class UMADamageTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetDamageText(float DamageAmount, bool bIsCritical, bool bIsPlayerHit);

protected:
	UPROPERTY(meta=(BindWidget))
	UTextBlock* DamageText;
	UPROPERTY(Transient, meta=(BindWidgetAnim))
	UWidgetAnimation* FadeUpAnim;
};
