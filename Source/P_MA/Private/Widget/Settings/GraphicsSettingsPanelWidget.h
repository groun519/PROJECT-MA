// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Settings/SettingsPanelWidgetBase.h"
#include "GraphicsSettingsPanelWidget.generated.h"

class USettingsDropdownRowWidget;
class USettingsToggleRowWidget;

UCLASS()
class P_MA_API UGraphicsSettingsPanelWidget : public USettingsPanelWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
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
	void ApplyDirectResolution();
	void SetScreenPercentage(float Value);
	FIntPoint GetDesktopResolution() const;
	void NudgeWindowedGameWindowDown() const;

	/** Preset **/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsDropdownRowWidget> PresetDropdownRow;

	int32 SelectedPresetIndex = 2;
	void InitPresetRow(const UGameUserSettings* Settings);
	void HandlePresetSelectionChanged(int32 InIndex);
	int32 ResolvePresetIndex(const UGameUserSettings* Settings) const;
	void ApplyPresetQualityLevel(UGameUserSettings* Settings, int32 InQualityLevel) const;

	/** Quality **/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsToggleRowWidget> ViewDistanceToggleRow;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsToggleRowWidget> ShadowToggleRow;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsToggleRowWidget> GlobalIlluminationToggleRow;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsToggleRowWidget> ReflectionToggleRow;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsToggleRowWidget> AntiAliasingToggleRow;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsToggleRowWidget> TextureToggleRow;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsToggleRowWidget> EffectToggleRow;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsToggleRowWidget> PostProcessingToggleRow;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsToggleRowWidget> FoliageToggleRow;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsToggleRowWidget> ShadingToggleRow;

	void InitQualityRows(const UGameUserSettings* Settings);
	void InitQualityRow(USettingsToggleRowWidget* Row, const FText& Label, int32 InQualityLevel) const;
	void ApplySingleQualityLevel(int32 InIndex, void (UGameUserSettings::*Setter)(int32));
	void HandleViewDistanceSelectionChanged(int32 InIndex);
	void HandleShadowSelectionChanged(int32 InIndex);
	void HandleGlobalIlluminationSelectionChanged(int32 InIndex);
	void HandleReflectionSelectionChanged(int32 InIndex);
	void HandleAntiAliasingSelectionChanged(int32 InIndex);
	void HandleTextureSelectionChanged(int32 InIndex);
	void HandleEffectSelectionChanged(int32 InIndex);
	void HandlePostProcessingSelectionChanged(int32 InIndex);
	void HandleFoliageSelectionChanged(int32 InIndex);
	void HandleShadingSelectionChanged(int32 InIndex);

	/** Max FPS **/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsDropdownRowWidget> MaxFpsDropdownRow;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsToggleRowWidget> VSyncToggleRow;

	int32 SelectedMaxFpsIndex = 1;
	int32 SelectedMaxFpsValue = 60;
	void InitMaxFpsRow(const UGameUserSettings* Settings);
	void HandleMaxFpsSelectionChanged(int32 InIndex);
	void InitVSyncRow(const UGameUserSettings* Settings);
	void HandleVSyncSelectionChanged(int32 InIndex);
};
