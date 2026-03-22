// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsKeyBindingRowWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSettingsKeyBindingRowRebindRequested, USettingsKeyBindingRowWidget*, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSettingsKeyBindingRowActionRequested, USettingsKeyBindingRowWidget*);

UCLASS()
class P_MA_API USettingsKeyBindingRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void SetupRow(const FText& InActionName, const FText& InKeyText);
	void SetupSecondaryKey(const FText& InKeyText);

	FOnSettingsKeyBindingRowRebindRequested OnRebindRequested;
	FOnSettingsKeyBindingRowActionRequested OnResetRequested;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ActionNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CurrentKeyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentKeyText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SecondaryKeyButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SecondaryKeyText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ResetButton;

private:
	void SetKeyText(UTextBlock* TargetText, const FText& InKeyText);
	void SetSecondaryKeyVisible(bool bVisible);

	UFUNCTION()
	void HandleCurrentKeyButtonClicked();

	UFUNCTION()
	void HandleSecondaryKeyButtonClicked();

	UFUNCTION()
	void HandleResetButtonClicked();
};
