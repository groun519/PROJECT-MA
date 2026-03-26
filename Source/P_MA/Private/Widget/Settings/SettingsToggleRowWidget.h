// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsToggleRowWidget.generated.h"

class UTextBlock;
class USettingsToggleButtonWidget;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSettingsToggleRowSelectionChanged, int32);

UCLASS()
class P_MA_API USettingsToggleRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void SetupOptions(const FText& InLabel, const TArray<FText>& InOptions, int32 InSelectedIndex);
	int32 GetSelectedIndex() const { return SelectedIndex; }

	FOnSettingsToggleRowSelectionChanged OnSelectionChanged;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LabelText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsToggleButtonWidget> ToggleButton_0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsToggleButtonWidget> ToggleButton_1;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USettingsToggleButtonWidget> ToggleButton_2;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USettingsToggleButtonWidget> ToggleButton_3;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USettingsToggleButtonWidget> ToggleButton_4;

private:
	TArray<TObjectPtr<USettingsToggleButtonWidget>> ToggleButtons;
	int32 ActiveButtonCount = 0;
	int32 SelectedIndex = 0;
	bool bUpdatingSelection = false;

	void CacheToggleButtons();
	void UpdateSelection();
	void HandleToggleClicked(int32 InIndex);
};
