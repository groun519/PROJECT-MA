// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Settings/SettingsPanelWidgetBase.h"
#include "AudioSettingsPanelWidget.generated.h"

class USettingsSliderRowWidget;

UCLASS()
class P_MA_API UAudioSettingsPanelWidget : public USettingsPanelWidgetBase
{
	GENERATED_BODY()

public:
	/** Lifecycle **/
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

private:
	/** Master Volume **/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsSliderRowWidget> MasterVolumeSliderRow;

	void InitMasterVolumeRow();
	void HandleMasterVolumeChanged(float InValue);
};
