// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/GraphicsSettingsPanelWidget.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/GameUserSettings.h"
#include "HAL/IConsoleManager.h"
#include "Player/MAPlayerControllerBase.h"
#include "Widgets/SWindow.h"
#include "Widget/Settings/SettingsDropdownRowWidget.h"
#include "Widget/Settings/SettingsToggleRowWidget.h"

namespace
{
	const TArray<FIntPoint> GResolutionValues =
	{
		FIntPoint(1280, 720),
		FIntPoint(1600, 900),
		FIntPoint(1920, 1080),
	};

	const TArray<int32> GMaxFpsValues = { 30, 60, 80, 120, 144, 0 };

	constexpr int32 GPresetCustomIndex = 5;

	const TArray<FText>& GetQualityOptions()
	{
		static const TArray<FText> QualityOptions =
		{
			NSLOCTEXT("GraphicsSettingsPanel", "QualityLow", "Low"),
			NSLOCTEXT("GraphicsSettingsPanel", "QualityMedium", "Medium"),
			NSLOCTEXT("GraphicsSettingsPanel", "QualityHigh", "High"),
			NSLOCTEXT("GraphicsSettingsPanel", "QualityEpic", "Epic"),
			NSLOCTEXT("GraphicsSettingsPanel", "QualityUltra", "Ultra")
		};

		return QualityOptions;
	}
}

void UGraphicsSettingsPanelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	WindowModeDropdownRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandleWindowModeSelectionChanged);
	ResolutionDropdownRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandleResolutionSelectionChanged);
	PresetDropdownRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandlePresetSelectionChanged);
	ViewDistanceToggleRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandleViewDistanceSelectionChanged);
	ShadowToggleRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandleShadowSelectionChanged);
	GlobalIlluminationToggleRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandleGlobalIlluminationSelectionChanged);
	ReflectionToggleRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandleReflectionSelectionChanged);
	AntiAliasingToggleRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandleAntiAliasingSelectionChanged);
	TextureToggleRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandleTextureSelectionChanged);
	EffectToggleRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandleEffectSelectionChanged);
	PostProcessingToggleRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandlePostProcessingSelectionChanged);
	FoliageToggleRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandleFoliageSelectionChanged);
	ShadingToggleRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandleShadingSelectionChanged);
	MaxFpsDropdownRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandleMaxFpsSelectionChanged);
	VSyncToggleRow->OnSelectionChanged.AddUObject(this, &UGraphicsSettingsPanelWidget::HandleVSyncSelectionChanged);
}

void UGraphicsSettingsPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	InitWindowModeRow(Settings);
	InitResolutionRow(Settings);
	InitPresetRow(Settings);
	InitQualityRows(Settings);
	InitMaxFpsRow(Settings);
	InitVSyncRow(Settings);
}

/** Window Mode **/
void UGraphicsSettingsPanelWidget::InitWindowModeRow(const UGameUserSettings* Settings)
{
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

	if (SelectedWindowModeIndex == 1)
	{
		const FIntPoint DesktopResolution = GetDesktopResolution();
		if (DesktopResolution.X > 0 && DesktopResolution.Y > 0)
		{
			Settings->SetScreenResolution(DesktopResolution);
		}
	}
	else
	{
		Settings->SetScreenResolution(SelectedResolution);
	}

	SetScreenPercentage(100.0f);
	ApplySettingsAndKeepFocus(Settings);
	if (SelectedWindowModeIndex == 0) NudgeWindowedGameWindowDown();
	InitResolutionRow(Settings);
}

void UGraphicsSettingsPanelWidget::ApplySettingsAndKeepFocus(UGameUserSettings* Settings)
{
	Settings->ApplySettings(false);

	if (AMAPlayerControllerBase* PC = GetOwningPlayer<AMAPlayerControllerBase>())
	{
		PC->RefreshSettingsFocus();
	}
}

/** Resolution **/
void UGraphicsSettingsPanelWidget::InitResolutionRow(const UGameUserSettings* Settings)
{
	TArray<FText> ResolutionOptions;
	ResolutionOptions.Reserve(GResolutionValues.Num());

	for (const FIntPoint& Value : GResolutionValues)
	{
		ResolutionOptions.Add(FText::FromString(FString::Printf(TEXT("%dx%d"), Value.X, Value.Y)));
	}

	FIntPoint CurrentResolution = Settings ? Settings->GetScreenResolution() : FIntPoint(1920, 1080);
	if (SelectedWindowModeIndex == 1)
	{
		const FIntPoint DesktopResolution = GetDesktopResolution();
		if (DesktopResolution.X > 0 && DesktopResolution.Y > 0)
		{
			CurrentResolution = DesktopResolution;
		}
	}
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
	ResolutionDropdownRow->SetIsEnabled(SelectedWindowModeIndex != 1);
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
	if (SelectedWindowModeIndex == 1) return;

	ApplyDirectResolution();
}

void UGraphicsSettingsPanelWidget::ApplyDirectResolution()
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings) return;

	Settings->SetScreenResolution(SelectedResolution);
	ApplySettingsAndKeepFocus(Settings);
	SetScreenPercentage(100.0f);
	NudgeWindowedGameWindowDown();
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
	const UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	return Settings ? Settings->GetDesktopResolution() : FIntPoint::ZeroValue;
}

void UGraphicsSettingsPanelWidget::NudgeWindowedGameWindowDown() const
{
	if (!FSlateApplication::IsInitialized()) return;
	if (!GEngine || !GEngine->GameViewport) return;

	const TSharedPtr<SWindow> GameWindow = GEngine->GameViewport->GetWindow();
	if (!GameWindow.IsValid() || GameWindow->GetWindowMode() != EWindowMode::Windowed) return;

	const FVector2D WindowPosition = GameWindow->GetPositionInScreen();
	const FVector2D WindowSize = GameWindow->GetSizeInScreen();
	const FSlateRect WorkArea = FSlateApplication::Get().GetWorkArea(FSlateRect::FromPointAndExtent(WindowPosition, WindowSize));
	const float TargetX = FMath::Max(WindowPosition.X, WorkArea.Left);
	const float TargetY = FMath::Max(WindowPosition.Y, WorkArea.Top + 24.0f);

	if (!FMath::IsNearlyEqual(TargetX, WindowPosition.X) || !FMath::IsNearlyEqual(TargetY, WindowPosition.Y))
	{
		GameWindow->MoveWindowTo(FVector2D(TargetX, TargetY));
	}
}

/** Preset **/
void UGraphicsSettingsPanelWidget::InitPresetRow(const UGameUserSettings* Settings)
{
	TArray<FText> PresetOptions;
	PresetOptions.Reserve(6);
	PresetOptions.Add(NSLOCTEXT("GraphicsSettingsPanel", "PresetLow", "Low"));
	PresetOptions.Add(NSLOCTEXT("GraphicsSettingsPanel", "PresetMedium", "Medium"));
	PresetOptions.Add(NSLOCTEXT("GraphicsSettingsPanel", "PresetHigh", "High"));
	PresetOptions.Add(NSLOCTEXT("GraphicsSettingsPanel", "PresetEpic", "Epic"));
	PresetOptions.Add(NSLOCTEXT("GraphicsSettingsPanel", "PresetUltra", "Ultra"));
	PresetOptions.Add(NSLOCTEXT("GraphicsSettingsPanel", "PresetCustom", "Custom"));

	SelectedPresetIndex = ResolvePresetIndex(Settings);

	PresetDropdownRow->SetupOptions(NSLOCTEXT("GraphicsSettingsPanel", "Preset", "Preset"), PresetOptions, SelectedPresetIndex);
}

void UGraphicsSettingsPanelWidget::HandlePresetSelectionChanged(int32 InIndex)
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings) return;
	if (InIndex == GPresetCustomIndex)
	{
		InitPresetRow(Settings);
		return;
	}

	SelectedPresetIndex = FMath::Clamp(InIndex, 0, GPresetCustomIndex - 1);

	ApplyPresetQualityLevel(Settings, SelectedPresetIndex);
	ApplySettingsAndKeepFocus(Settings);
	InitQualityRows(Settings);
}

int32 UGraphicsSettingsPanelWidget::ResolvePresetIndex(const UGameUserSettings* Settings) const
{
	if (!Settings) return 2;

	const int32 QualityLevel = Settings->GetViewDistanceQuality();
	const bool bMatchesSingleLevel =
		Settings->GetShadowQuality() == QualityLevel &&
		Settings->GetGlobalIlluminationQuality() == QualityLevel &&
		Settings->GetReflectionQuality() == QualityLevel &&
		Settings->GetAntiAliasingQuality() == QualityLevel &&
		Settings->GetTextureQuality() == QualityLevel &&
		Settings->GetVisualEffectQuality() == QualityLevel &&
		Settings->GetPostProcessingQuality() == QualityLevel &&
		Settings->GetFoliageQuality() == QualityLevel &&
		Settings->GetShadingQuality() == QualityLevel;

	return bMatchesSingleLevel ? FMath::Clamp(QualityLevel, 0, GPresetCustomIndex - 1) : GPresetCustomIndex;
}

void UGraphicsSettingsPanelWidget::ApplyPresetQualityLevel(UGameUserSettings* Settings, int32 InQualityLevel) const
{
	const int32 QualityLevel = FMath::Clamp(InQualityLevel, 0, 4);
	Settings->SetViewDistanceQuality(QualityLevel);
	Settings->SetShadowQuality(QualityLevel);
	Settings->SetGlobalIlluminationQuality(QualityLevel);
	Settings->SetReflectionQuality(QualityLevel);
	Settings->SetAntiAliasingQuality(QualityLevel);
	Settings->SetTextureQuality(QualityLevel);
	Settings->SetVisualEffectQuality(QualityLevel);
	Settings->SetPostProcessingQuality(QualityLevel);
	Settings->SetFoliageQuality(QualityLevel);
	Settings->SetShadingQuality(QualityLevel);
}

/** Quality **/
void UGraphicsSettingsPanelWidget::InitQualityRows(const UGameUserSettings* Settings)
{
	InitQualityRow(ViewDistanceToggleRow, NSLOCTEXT("GraphicsSettingsPanel", "ViewDistance", "View Distance"), Settings ? Settings->GetViewDistanceQuality() : 2);
	InitQualityRow(ShadowToggleRow, NSLOCTEXT("GraphicsSettingsPanel", "Shadows", "Shadows"), Settings ? Settings->GetShadowQuality() : 2);
	InitQualityRow(GlobalIlluminationToggleRow, NSLOCTEXT("GraphicsSettingsPanel", "GlobalIllumination", "Global Illumination"), Settings ? Settings->GetGlobalIlluminationQuality() : 2);
	InitQualityRow(ReflectionToggleRow, NSLOCTEXT("GraphicsSettingsPanel", "Reflections", "Reflections"), Settings ? Settings->GetReflectionQuality() : 2);
	InitQualityRow(AntiAliasingToggleRow, NSLOCTEXT("GraphicsSettingsPanel", "AntiAliasing", "Anti-Aliasing"), Settings ? Settings->GetAntiAliasingQuality() : 2);
	InitQualityRow(TextureToggleRow, NSLOCTEXT("GraphicsSettingsPanel", "Textures", "Textures"), Settings ? Settings->GetTextureQuality() : 2);
	InitQualityRow(EffectToggleRow, NSLOCTEXT("GraphicsSettingsPanel", "Effects", "Effects"), Settings ? Settings->GetVisualEffectQuality() : 2);
	InitQualityRow(PostProcessingToggleRow, NSLOCTEXT("GraphicsSettingsPanel", "PostProcessing", "Post Processing"), Settings ? Settings->GetPostProcessingQuality() : 2);
	InitQualityRow(FoliageToggleRow, NSLOCTEXT("GraphicsSettingsPanel", "Foliage", "Foliage"), Settings ? Settings->GetFoliageQuality() : 2);
	InitQualityRow(ShadingToggleRow, NSLOCTEXT("GraphicsSettingsPanel", "Shading", "Shading"), Settings ? Settings->GetShadingQuality() : 2);
}

void UGraphicsSettingsPanelWidget::InitQualityRow(USettingsToggleRowWidget* Row, const FText& Label, int32 InQualityLevel) const
{
	Row->SetupOptions(Label, GetQualityOptions(), FMath::Clamp(InQualityLevel, 0, GPresetCustomIndex - 1));
}

void UGraphicsSettingsPanelWidget::ApplySingleQualityLevel(int32 InIndex, void (UGameUserSettings::*Setter)(int32))
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings) return;

	(Settings->*Setter)(FMath::Clamp(InIndex, 0, GPresetCustomIndex - 1));
	ApplySettingsAndKeepFocus(Settings);
	InitPresetRow(Settings);
}

void UGraphicsSettingsPanelWidget::HandleViewDistanceSelectionChanged(int32 InIndex)
{
	ApplySingleQualityLevel(InIndex, &UGameUserSettings::SetViewDistanceQuality);
}

void UGraphicsSettingsPanelWidget::HandleShadowSelectionChanged(int32 InIndex)
{
	ApplySingleQualityLevel(InIndex, &UGameUserSettings::SetShadowQuality);
}

void UGraphicsSettingsPanelWidget::HandleGlobalIlluminationSelectionChanged(int32 InIndex)
{
	ApplySingleQualityLevel(InIndex, &UGameUserSettings::SetGlobalIlluminationQuality);
}

void UGraphicsSettingsPanelWidget::HandleReflectionSelectionChanged(int32 InIndex)
{
	ApplySingleQualityLevel(InIndex, &UGameUserSettings::SetReflectionQuality);
}

void UGraphicsSettingsPanelWidget::HandleAntiAliasingSelectionChanged(int32 InIndex)
{
	ApplySingleQualityLevel(InIndex, &UGameUserSettings::SetAntiAliasingQuality);
}

void UGraphicsSettingsPanelWidget::HandleTextureSelectionChanged(int32 InIndex)
{
	ApplySingleQualityLevel(InIndex, &UGameUserSettings::SetTextureQuality);
}

void UGraphicsSettingsPanelWidget::HandleEffectSelectionChanged(int32 InIndex)
{
	ApplySingleQualityLevel(InIndex, &UGameUserSettings::SetVisualEffectQuality);
}

void UGraphicsSettingsPanelWidget::HandlePostProcessingSelectionChanged(int32 InIndex)
{
	ApplySingleQualityLevel(InIndex, &UGameUserSettings::SetPostProcessingQuality);
}

void UGraphicsSettingsPanelWidget::HandleFoliageSelectionChanged(int32 InIndex)
{
	ApplySingleQualityLevel(InIndex, &UGameUserSettings::SetFoliageQuality);
}

void UGraphicsSettingsPanelWidget::HandleShadingSelectionChanged(int32 InIndex)
{
	ApplySingleQualityLevel(InIndex, &UGameUserSettings::SetShadingQuality);
}

/** Max FPS **/
void UGraphicsSettingsPanelWidget::InitMaxFpsRow(const UGameUserSettings* Settings)
{
	TArray<FText> Options;
	Options.Reserve(GMaxFpsValues.Num());

	for (int32 Value : GMaxFpsValues)
	{
		Options.Add(Value > 0 ? FText::AsNumber(Value) : NSLOCTEXT("GraphicsSettingsPanel", "Unlimited", "Unlimited"));
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

/** VSync **/
void UGraphicsSettingsPanelWidget::InitVSyncRow(const UGameUserSettings* Settings)
{
	const bool bVSyncEnabled = Settings ? Settings->IsVSyncEnabled() : false;
	const int32 SelectedIndex = bVSyncEnabled ? 0 : 1;
	const TArray<FText> Options =
	{
		NSLOCTEXT("GraphicsSettingsPanel", "VSyncOn", "On"),
		NSLOCTEXT("GraphicsSettingsPanel", "VSyncOff", "Off")
	};

	VSyncToggleRow->SetupOptions(NSLOCTEXT("GraphicsSettingsPanel", "VSync", "VSync"), Options, SelectedIndex);
}

void UGraphicsSettingsPanelWidget::HandleVSyncSelectionChanged(int32 InIndex)
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings) return;

	const bool bEnable = (InIndex == 0);
	Settings->SetVSyncEnabled(bEnable);
	ApplySettingsAndKeepFocus(Settings);
}
