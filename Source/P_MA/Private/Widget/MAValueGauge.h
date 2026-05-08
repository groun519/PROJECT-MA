// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "MAValueGauge.generated.h"

/**
 *
 */
UCLASS()
class UMAValueGauge : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;
	void SetAndBoundToGameplayAttribute(class UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAttribute& Attribute, const FGameplayAttribute& MaxAttribute);
	void SetValue(float NewValue, float NewMaxValue);

private:
	void ValueChanged(const FOnAttributeChangeData& ChangedData);
	void MaxValueChanged(const FOnAttributeChangeData& ChangedData);

	float CachedValue;
	float CachedMaxValue;

	UPROPERTY(EditAnywhere, Category = "Visual")
	FLinearColor BarColor;

	UPROPERTY(EditAnywhere, Category = "Visual")
	FSlateFontInfo ValueTextFont;

	UPROPERTY(EditAnywhere, Category = "Visual")
	bool bValueTextVisible = true;
	
	UPROPERTY(EditAnywhere, Category = "Visual")
	bool bProgressBarVisible = true;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
	class UTextBlock* ValueText;
	
	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
	class UProgressBar* GhostProgressBar;

	FTimerHandle GhostTimerHandle;
	void UpdateGhostBar();

	float TargetPercent = 1.0f;
	float CurrentGhostPercent = 1.0f;
};
