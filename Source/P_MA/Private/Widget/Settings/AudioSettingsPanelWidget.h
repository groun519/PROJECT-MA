// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Settings/SettingsPanelWidgetBase.h"
#include "AudioSettingsPanelWidget.generated.h"

class USettingsSliderRowWidget;
class UMAAudioSettingsSubsystem;

UCLASS()
class P_MA_API UAudioSettingsPanelWidget : public USettingsPanelWidgetBase
{
	GENERATED_BODY()

public:
	/** Lifecycle **/
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

private:
	/** Audio **/
	UMAAudioSettingsSubsystem* GetAudioSettingsSubsystem() const;

	/** Master Volume **/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsSliderRowWidget> MasterVolumeSliderRow;

	/** BGM Volume **/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsSliderRowWidget> BgmVolumeSliderRow;

	/** SFX Volume **/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsSliderRowWidget> SfxVolumeSliderRow;

	/** UI Volume **/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsSliderRowWidget> UiVolumeSliderRow;

	void InitMasterVolumeRow();
	void InitBgmVolumeRow();
	void InitSfxVolumeRow();
	void InitUiVolumeRow();
	void HandleMasterVolumeChanged(float InValue);
	void HandleBgmVolumeChanged(float InValue);
	void HandleSfxVolumeChanged(float InValue);
	void HandleUiVolumeChanged(float InValue);
};
