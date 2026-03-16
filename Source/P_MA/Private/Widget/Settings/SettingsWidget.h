// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Settings/SettingsCategoryButtonWidget.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

class UButton;
class UWidgetSwitcher;

UCLASS()
class P_MA_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetActiveCategory(ESettingsCategory NewCategory);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Settings")
	void HandleApplyRequested();

	UFUNCTION(BlueprintImplementableEvent, Category = "Settings")
	void HandleCancelRequested();

	UFUNCTION(BlueprintImplementableEvent, Category = "Settings")
	void HandleDefaultRequested();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsCategoryButtonWidget> GraphicsCategoryButton_0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsCategoryButtonWidget> AudioCategoryButton_1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsCategoryButtonWidget> ControlsCategoryButton_2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsCategoryButtonWidget> GameplayCategoryButton_3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> SettingsPanelSwitcher;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ApplyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CancelButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DefaultButton;

private:
	void UpdateCategorySelection(ESettingsCategory ActiveCategory);

	UFUNCTION()
	void HandleCategoryButtonClicked(ESettingsCategory Category);
};
