// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Settings/SettingsPanelWidgetBase.h"
#include "GameplaySettingsPanelWidget.generated.h"

class USettingsDropdownRowWidget;

UCLASS()
class P_MA_API UGameplaySettingsPanelWidget : public USettingsPanelWidgetBase
{
	GENERATED_BODY()

public:
	/** Lifecycle **/
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

private:
	/** Language **/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsDropdownRowWidget> LanguageDropdownRow;

	void InitLanguageRow();
	void HandleLanguageSelectionChanged(int32 InIndex);
};
