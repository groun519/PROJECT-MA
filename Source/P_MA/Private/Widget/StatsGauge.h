// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "StatsGauge.generated.h"

class UMaterialInterface;

/**
 * 
 */
UCLASS()
class UStatsGauge : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
private:
	UPROPERTY(meta=(BindWidget))
	class UImage* Icon;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* AttributeText;

	UPROPERTY(EditAnywhere, Category = "Attribute")
	FGameplayAttribute Attribute;

	UPROPERTY(EditAnywhere, Category = "Visual")
	TObjectPtr<UMaterialInterface> IconMaterial;

	void SetValue(float NewVal);
	FNumberFormattingOptions NumberFormattingOptions;

	void AttributeChanged(const FOnAttributeChangeData& Data);
};