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
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetActiveCategory(ESettingsCategory NewCategory);

protected:
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

private:
	void UpdateCategorySelection(ESettingsCategory ActiveCategory);

	UFUNCTION()
	void HandleCategoryButtonClicked(ESettingsCategory Category);
};
