// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/AudioSettingsPanelWidget.h"

#include "Framework/MAGameInstance.h"
#include "Widget/Settings/SettingsSliderRowWidget.h"

/** Lifecycle **/
void UAudioSettingsPanelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	MasterVolumeSliderRow->OnValueChanged.AddUObject(this, &UAudioSettingsPanelWidget::HandleMasterVolumeChanged);
}

void UAudioSettingsPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitMasterVolumeRow();
}

/** Master Volume **/
void UAudioSettingsPanelWidget::InitMasterVolumeRow()
{
	const UMAGameInstance* GameInstance = GetGameInstance<UMAGameInstance>();
	const float MasterVolume = GameInstance ? GameInstance->GetCurrentMasterVolume() : 1.0f;

	MasterVolumeSliderRow->SetupValue(NSLOCTEXT("AudioSettingsPanel", "MasterVolume", "Master Volume"), MasterVolume);
}

void UAudioSettingsPanelWidget::HandleMasterVolumeChanged(float InValue)
{
	if (UMAGameInstance* GameInstance = GetGameInstance<UMAGameInstance>())
	{
		GameInstance->SetCurrentMasterVolume(InValue);
	}
}
