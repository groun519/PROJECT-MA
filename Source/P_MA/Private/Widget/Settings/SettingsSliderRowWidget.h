// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsSliderRowWidget.generated.h"

class USlider;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSettingsSliderRowValueChanged, float);

UCLASS()
class P_MA_API USettingsSliderRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void SetupValue(const FText& InLabel, float InValue);
	void SetValue(float InValue);
	float GetValue() const { return Value; }

	FOnSettingsSliderRowValueChanged OnValueChanged;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LabelText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> ValueSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ValueText;

private:
	float Value = 1.0f;
	bool bUpdatingValue = false;

	void RefreshValue();
	void RefreshValueText();

	UFUNCTION()
	void HandleSliderValueChanged(float InValue);
};
