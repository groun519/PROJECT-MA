// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/AudioSettingsPanelWidget.h"

#include "Audio/MAAudioSettingsSubsystem.h"
#include "Widget/Settings/SettingsSliderRowWidget.h"

/** Lifecycle **/
void UAudioSettingsPanelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	MasterVolumeSliderRow->OnValueChanged.AddUObject(this, &UAudioSettingsPanelWidget::HandleMasterVolumeChanged);
	BgmVolumeSliderRow->OnValueChanged.AddUObject(this, &UAudioSettingsPanelWidget::HandleBgmVolumeChanged);
	SfxVolumeSliderRow->OnValueChanged.AddUObject(this, &UAudioSettingsPanelWidget::HandleSfxVolumeChanged);
	UiVolumeSliderRow->OnValueChanged.AddUObject(this, &UAudioSettingsPanelWidget::HandleUiVolumeChanged);
}

void UAudioSettingsPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitMasterVolumeRow();
	InitBgmVolumeRow();
	InitSfxVolumeRow();
	InitUiVolumeRow();
}

/** Audio **/
UMAAudioSettingsSubsystem* UAudioSettingsPanelWidget::GetAudioSettingsSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UMAAudioSettingsSubsystem>();
	}

	return nullptr;
}

/** Master Volume **/
void UAudioSettingsPanelWidget::InitMasterVolumeRow()
{
	const UMAAudioSettingsSubsystem* AudioSubsystem = GetAudioSettingsSubsystem();
	const float MasterVolume = AudioSubsystem ? AudioSubsystem->GetCurrentMasterVolume() : 1.0f;

	MasterVolumeSliderRow->SetupValue(NSLOCTEXT("AudioSettingsPanel", "MasterVolume", "Master Volume"), MasterVolume);
}

void UAudioSettingsPanelWidget::HandleMasterVolumeChanged(float InValue)
{
	if (UMAAudioSettingsSubsystem* AudioSubsystem = GetAudioSettingsSubsystem())
	{
		AudioSubsystem->SetCurrentMasterVolume(InValue);
	}
}

/** BGM Volume **/
void UAudioSettingsPanelWidget::InitBgmVolumeRow()
{
	const UMAAudioSettingsSubsystem* AudioSubsystem = GetAudioSettingsSubsystem();
	const float BgmVolume = AudioSubsystem ? AudioSubsystem->GetCurrentBgmVolume() : 1.0f;

	BgmVolumeSliderRow->SetupValue(NSLOCTEXT("AudioSettingsPanel", "BgmVolume", "BGM Volume"), BgmVolume);
}

void UAudioSettingsPanelWidget::HandleBgmVolumeChanged(float InValue)
{
	if (UMAAudioSettingsSubsystem* AudioSubsystem = GetAudioSettingsSubsystem())
	{
		AudioSubsystem->SetCurrentBgmVolume(InValue);
	}
}

/** SFX Volume **/
void UAudioSettingsPanelWidget::InitSfxVolumeRow()
{
	const UMAAudioSettingsSubsystem* AudioSubsystem = GetAudioSettingsSubsystem();
	const float SfxVolume = AudioSubsystem ? AudioSubsystem->GetCurrentSfxVolume() : 1.0f;

	SfxVolumeSliderRow->SetupValue(NSLOCTEXT("AudioSettingsPanel", "SfxVolume", "SFX Volume"), SfxVolume);
}

void UAudioSettingsPanelWidget::HandleSfxVolumeChanged(float InValue)
{
	if (UMAAudioSettingsSubsystem* AudioSubsystem = GetAudioSettingsSubsystem())
	{
		AudioSubsystem->SetCurrentSfxVolume(InValue);
	}
}

/** UI Volume **/
void UAudioSettingsPanelWidget::InitUiVolumeRow()
{
	const UMAAudioSettingsSubsystem* AudioSubsystem = GetAudioSettingsSubsystem();
	const float UiVolume = AudioSubsystem ? AudioSubsystem->GetCurrentUiVolume() : 1.0f;

	UiVolumeSliderRow->SetupValue(NSLOCTEXT("AudioSettingsPanel", "UiVolume", "UI Volume"), UiVolume);
}

void UAudioSettingsPanelWidget::HandleUiVolumeChanged(float InValue)
{
	if (UMAAudioSettingsSubsystem* AudioSubsystem = GetAudioSettingsSubsystem())
	{
		AudioSubsystem->SetCurrentUiVolume(InValue);
	}
}
