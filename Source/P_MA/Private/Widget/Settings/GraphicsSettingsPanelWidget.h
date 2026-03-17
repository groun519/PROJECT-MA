// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Settings/SettingsPanelWidgetBase.h"
#include "GraphicsSettingsPanelWidget.generated.h"

class USettingsDropdownRowWidget;

UCLASS()
class P_MA_API UGraphicsSettingsPanelWidget : public USettingsPanelWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

private:
	/** Window Mode **/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsDropdownRowWidget> WindowModeDropdownRow;

	int32 SelectedWindowModeIndex = 0;
	void InitWindowModeRow(const UGameUserSettings* Settings);
	void HandleWindowModeSelectionChanged(int32 InIndex);
	void ApplySettingsAndKeepFocus(UGameUserSettings* Settings);

	/** Resolution **/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsDropdownRowWidget> ResolutionDropdownRow;

	int32 SelectedResolutionIndex = 2;
	FIntPoint SelectedResolution = FIntPoint(1920, 1080);
	void InitResolutionRow(const UGameUserSettings* Settings);
	void HandleResolutionSelectionChanged(int32 InIndex);
	void ApplyResolutionForCurrentMode();
	void ApplyFullscreenResolutionScale();
	void ApplyWindowedResolution();
	void SetScreenPercentage(float Value);
	FIntPoint GetDesktopResolution() const;

	/** Preset **/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsDropdownRowWidget> PresetDropdownRow;

	int32 SelectedPresetIndex = 2;
	void InitPresetRow(const UGameUserSettings* Settings);
	void HandlePresetSelectionChanged(int32 InIndex);

	/** Max FPS **/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsDropdownRowWidget> MaxFpsDropdownRow;

	int32 SelectedMaxFpsIndex = 1;
	int32 SelectedMaxFpsValue = 60;
	void InitMaxFpsRow(const UGameUserSettings* Settings);
	void HandleMaxFpsSelectionChanged(int32 InIndex);
};
