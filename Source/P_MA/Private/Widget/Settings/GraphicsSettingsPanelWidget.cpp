// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/GraphicsSettingsPanelWidget.h"

#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "GenericPlatform/GenericApplication.h"
#include "HAL/IConsoleManager.h"
#include "Player/MAPlayerControllerBase.h"
#include "Widget/Settings/SettingsDropdownRowWidget.h"

namespace
{
	const TArray<FIntPoint> GResolutionValues =
	{
		FIntPoint(800, 450),
		FIntPoint(960, 540),
		FIntPoint(1280, 720),
		FIntPoint(1600, 900),
		FIntPoint(1920, 1080),
	};

	const TArray<int32> GMaxFpsValues = { 30, 60, 80, 120, 144, 0 };
}

void UGraphicsSettingsPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	InitWindowModeRow(Settings);
	InitResolutionRow(Settings);
	InitPresetRow(Settings);
	InitMaxFpsRow(Settings);
}

/** Window Mode **/
void UGraphicsSettingsPanelWidget::InitWindowModeRow(const UGameUserSettings* Settings)
{
	if (!WindowModeDropdownRow) return;

	TArray<FText> WindowModeOptions;
	WindowModeOptions.Reserve(3);
	WindowModeOptions.Add(NSLOCTEXT("GraphicsSettingsPanel", "Windowed", "Windowed"));
	WindowModeOptions.Add(NSLOCTEXT("GraphicsSettingsPanel", "WindowedFullscreen", "Windowed Fullscreen"));
	WindowModeOptions.Add(NSLOCTEXT("GraphicsSettingsPanel", "Fullscreen", "Fullscreen"));

	const EWindowMode::Type CurrentMode = Settings ? Settings->GetFullscreenMode() : EWindowMode::Windowed;
	switch (CurrentMode)
	{
	case EWindowMode::WindowedFullscreen:
		SelectedWindowModeIndex = 1;
		break;
	case EWindowMode::Fullscreen:
		SelectedWindowModeIndex = 2;
		break;
	default:
		SelectedWindowModeIndex = 0;
		break;
	}

	WindowModeDropdownRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandleWindowModeSelectionChanged);
	WindowModeDropdownRow->SetupOptions(NSLOCTEXT("GraphicsSettingsPanel", "WindowMode", "Window Mode"), WindowModeOptions, SelectedWindowModeIndex);
}

void UGraphicsSettingsPanelWidget::HandleWindowModeSelectionChanged(int32 InIndex)
{
	SelectedWindowModeIndex = InIndex;

	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings) return;

	switch (InIndex)
	{
	case 1:
		Settings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
		break;
	case 2:
		Settings->SetFullscreenMode(EWindowMode::Fullscreen);
		break;
	default:
		Settings->SetFullscreenMode(EWindowMode::Windowed);
		break;
	}

	if (SelectedWindowModeIndex == 1 || SelectedWindowModeIndex == 2)
	{
		const FIntPoint DesktopResolution = GetDesktopResolution();
		if (DesktopResolution.X > 0 && DesktopResolution.Y > 0)
		{
			Settings->SetScreenResolution(DesktopResolution);
		}
	}

	ApplySettingsAndKeepFocus(Settings);
	ApplyResolutionForCurrentMode();
}

void UGraphicsSettingsPanelWidget::ApplySettingsAndKeepFocus(UGameUserSettings* Settings)
{
	if (!Settings) return;

	Settings->ApplySettings(false);
	Settings->SaveSettings();

	if (AMAPlayerControllerBase* PC = GetOwningPlayer<AMAPlayerControllerBase>())
	{
		PC->RefreshSettingsFocus();
	}
}

/** Resolution **/
void UGraphicsSettingsPanelWidget::InitResolutionRow(const UGameUserSettings* Settings)
{
	if (!ResolutionDropdownRow) return;

	TArray<FText> ResolutionOptions;
	ResolutionOptions.Reserve(GResolutionValues.Num());

	for (const FIntPoint& Value : GResolutionValues)
	{
		ResolutionOptions.Add(FText::FromString(FString::Printf(TEXT("%dx%d"), Value.X, Value.Y)));
	}

	FIntPoint BaseResolution = Settings ? Settings->GetScreenResolution() : FIntPoint(1920, 1080);
	if (SelectedWindowModeIndex == 1)
	{
		const FIntPoint DesktopResolution = GetDesktopResolution();
		if (DesktopResolution.X > 0 && DesktopResolution.Y > 0)
		{
			BaseResolution = DesktopResolution;
		}
	}

	float ScreenPercentage = 100.0f;
	if (SelectedWindowModeIndex != 0)
	{
		if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage")))
		{
			ScreenPercentage = CVar->GetFloat();
		}
	}

	const float Scale = FMath::Clamp(ScreenPercentage * 0.01f, 0.1f, 1.0f);
	const FIntPoint CurrentResolution(
		FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseResolution.X) * Scale)),
		FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseResolution.Y) * Scale))
	);
	SelectedResolutionIndex = GResolutionValues.IndexOfByKey(CurrentResolution);

	if (SelectedResolutionIndex == INDEX_NONE)
	{
		int32 BestIndex = 2;
		int32 BestDelta = TNumericLimits<int32>::Max();

		for (int32 Index = 0; Index < GResolutionValues.Num(); ++Index)
		{
			const FIntPoint& Value = GResolutionValues[Index];
			const int32 Delta = FMath::Abs(CurrentResolution.X - Value.X) + FMath::Abs(CurrentResolution.Y - Value.Y);
			if (Delta < BestDelta)
			{
				BestDelta = Delta;
				BestIndex = Index;
			}
		}

		SelectedResolutionIndex = BestIndex;
	}

	SelectedResolution = GResolutionValues.IsValidIndex(SelectedResolutionIndex) ? GResolutionValues[SelectedResolutionIndex] : FIntPoint(1920, 1080);
	ResolutionDropdownRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandleResolutionSelectionChanged);
	ResolutionDropdownRow->SetupOptions(NSLOCTEXT("GraphicsSettingsPanel", "Resolution", "Resolution"), ResolutionOptions, SelectedResolutionIndex);
}

void UGraphicsSettingsPanelWidget::HandleResolutionSelectionChanged(int32 InIndex)
{
	if (!GResolutionValues.IsValidIndex(InIndex)) return;

	SelectedResolutionIndex = InIndex;
	SelectedResolution = GResolutionValues[InIndex];

	ApplyResolutionForCurrentMode();
}

void UGraphicsSettingsPanelWidget::ApplyResolutionForCurrentMode()
{
	if (SelectedWindowModeIndex != 0)
	{
		ApplyFullscreenResolutionScale();
		return;
	}

	ApplyWindowedResolution();
}

void UGraphicsSettingsPanelWidget::ApplyFullscreenResolutionScale()
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings) return;

	const FIntPoint BaseResolution = (SelectedWindowModeIndex == 1) ? GetDesktopResolution() : Settings->GetScreenResolution();
	if (BaseResolution.X <= 0 || BaseResolution.Y <= 0) return;

	const float ScaleX = static_cast<float>(SelectedResolution.X) / static_cast<float>(BaseResolution.X);
	const float ScaleY = static_cast<float>(SelectedResolution.Y) / static_cast<float>(BaseResolution.Y);
	const float TargetScale = FMath::Clamp(FMath::Min(ScaleX, ScaleY) * 100.0f, 50.0f, 100.0f);

	SetScreenPercentage(TargetScale);
	Settings->SaveSettings();
}

void UGraphicsSettingsPanelWidget::ApplyWindowedResolution()
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings) return;

	Settings->SetScreenResolution(SelectedResolution);
	ApplySettingsAndKeepFocus(Settings);
	SetScreenPercentage(100.0f);
}

void UGraphicsSettingsPanelWidget::SetScreenPercentage(float Value)
{
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage")))
	{
		CVar->Set(Value, ECVF_SetByGameSetting);
	}
}

FIntPoint UGraphicsSettingsPanelWidget::GetDesktopResolution() const
{
	FDisplayMetrics Metrics;
	FDisplayMetrics::RebuildDisplayMetrics(Metrics);
	return FIntPoint(Metrics.PrimaryDisplayWidth, Metrics.PrimaryDisplayHeight);
}

/** Preset **/
void UGraphicsSettingsPanelWidget::InitPresetRow(const UGameUserSettings* Settings)
{
	if (!PresetDropdownRow) return;

	TArray<FText> PresetOptions;
	PresetOptions.Reserve(5);
	PresetOptions.Add(NSLOCTEXT("GraphicsSettingsPanel", "PresetLow", "Low"));
	PresetOptions.Add(NSLOCTEXT("GraphicsSettingsPanel", "PresetMedium", "Medium"));
	PresetOptions.Add(NSLOCTEXT("GraphicsSettingsPanel", "PresetHigh", "High"));
	PresetOptions.Add(NSLOCTEXT("GraphicsSettingsPanel", "PresetEpic", "Epic"));
	PresetOptions.Add(NSLOCTEXT("GraphicsSettingsPanel", "PresetCinematic", "Cinematic"));

	const int32 CurrentPreset = Settings ? Settings->GetOverallScalabilityLevel() : 2;
	SelectedPresetIndex = FMath::Clamp(CurrentPreset, 0, 4);

	PresetDropdownRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandlePresetSelectionChanged);
	PresetDropdownRow->SetupOptions(NSLOCTEXT("GraphicsSettingsPanel", "Preset", "Preset"), PresetOptions, SelectedPresetIndex);
}

void UGraphicsSettingsPanelWidget::HandlePresetSelectionChanged(int32 InIndex)
{
	SelectedPresetIndex = FMath::Clamp(InIndex, 0, 4);

	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings) return;

	Settings->SetOverallScalabilityLevel(SelectedPresetIndex);
	ApplySettingsAndKeepFocus(Settings);
}

/** Max FPS **/
void UGraphicsSettingsPanelWidget::InitMaxFpsRow(const UGameUserSettings* Settings)
{
	if (!MaxFpsDropdownRow) return;

	TArray<FText> Options;
	Options.Reserve(GMaxFpsValues.Num());

	for (int32 Value : GMaxFpsValues)
	{
		Options.Add(Value > 0 ? FText::AsNumber(Value) : FText::FromString(TEXT("Unlimited")));
	}

	const float CurrentLimit = Settings ? Settings->GetFrameRateLimit() : 60.0f;

	if (CurrentLimit <= 0.0f)
	{
		SelectedMaxFpsIndex = GMaxFpsValues.IndexOfByKey(0);
	}
	else
	{
		int32 BestIndex = 1;
		float BestDelta = TNumericLimits<float>::Max();

		for (int32 Index = 0; Index < GMaxFpsValues.Num(); ++Index)
		{
			const int32 Value = GMaxFpsValues[Index];
			if (Value <= 0) continue;

			const float Delta = FMath::Abs(CurrentLimit - static_cast<float>(Value));
			if (Delta < BestDelta)
			{
				BestDelta = Delta;
				BestIndex = Index;
			}
		}

		SelectedMaxFpsIndex = BestIndex;
	}

	SelectedMaxFpsValue = GMaxFpsValues.IsValidIndex(SelectedMaxFpsIndex) ? GMaxFpsValues[SelectedMaxFpsIndex] : 60;

	MaxFpsDropdownRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandleMaxFpsSelectionChanged);
	MaxFpsDropdownRow->SetupOptions(NSLOCTEXT("GraphicsSettingsPanel", "MaxFps", "Max FPS"), Options, SelectedMaxFpsIndex);
}

void UGraphicsSettingsPanelWidget::HandleMaxFpsSelectionChanged(int32 InIndex)
{
	if (!GMaxFpsValues.IsValidIndex(InIndex)) return;

	SelectedMaxFpsIndex = InIndex;
	SelectedMaxFpsValue = GMaxFpsValues[InIndex];

	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings) return;

	Settings->SetFrameRateLimit(static_cast<float>(SelectedMaxFpsValue));
	ApplySettingsAndKeepFocus(Settings);
}
