// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Settings/SettingsPanelWidgetBase.h"
#include "ControlsSettingsPanelWidget.generated.h"

class UInputMappingContext;
class USettingsKeyBindingRowWidget;
class USettingsSectionHeaderWidget;
class UVerticalBox;

UCLASS()
class P_MA_API UControlsSettingsPanelWidget : public USettingsPanelWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TArray<TObjectPtr<UInputMappingContext>> SourceContexts;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSubclassOf<USettingsKeyBindingRowWidget> KeyBindingRowClass;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSubclassOf<USettingsSectionHeaderWidget> CategoryHeaderWidgetClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> BindingRowsBox;

	void RebuildBindingRows();
};
