// Fill out your copyright notice in the Description page of Project Settings.

#include "LoadingScreenWidget.h"
#include "Widget/Lobby/Loading/LoadingBackgroundData.h"
#include "Widget/Lobby/Loading/LoadingPlayerStatusWidget.h"
#include "Widget/Lobby/Loading/LoadingTooltipData.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void ULoadingScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	DisplayProgress = 0.0f;
	LastUpdateSeconds = FPlatformTime::Seconds();
	FirstSeenSeconds = LastUpdateSeconds;
	bFinishPhase = false;
	FinishStartSeconds = 0.0;
	PendingTargetProgress = 0.0f;
	bPendingLoadingComplete = false;
	bHasPendingProgress = false;

	if (LoadingBackgroundImage && BackgroundData && BackgroundData->BackgroundImages.Num() > 0)
	{
		const int32 Index = FMath::RandRange(0, BackgroundData->BackgroundImages.Num() - 1);
		if (BackgroundData->BackgroundImages.IsValidIndex(Index) && BackgroundData->BackgroundImages[Index])
		{
			LoadingBackgroundImage->SetBrushFromTexture(BackgroundData->BackgroundImages[Index], true);
		}
	}

	if (LoadingTooltipText && TooltipData && TooltipData->Tips.Num() > 0)
	{
		const int32 Index = FMath::RandRange(0, TooltipData->Tips.Num() - 1);
		if (TooltipData->Tips.IsValidIndex(Index))
		{
			LoadingTooltipText->SetText(TooltipData->Tips[Index]);
		}
	}
}

void ULoadingScreenWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bHasPendingProgress)
	{
		return;
	}

	ApplyProgressFromTarget(PendingTargetProgress, bPendingLoadingComplete);
}

void ULoadingScreenWidget::UpdateLoadingStatus(const TArray<FLoadingPlayerStatus>& Statuses)
{
	if (!PlayerStatusBox) return;

	const int32 Count = Statuses.Num();
	EnsureEntryWidgets(Count);


	for (int32 Index = 0; Index < Count; ++Index)
	{
		if (StatusWidgets.IsValidIndex(Index) && StatusWidgets[Index])
		{
			StatusWidgets[Index]->SetStatus(Statuses[Index]);
		}
	}
}

void ULoadingScreenWidget::EnsureEntryWidgets(int32 Count)
{
	const bool bNeedsRebuild =
		(StatusWidgets.Num() != Count);

	if (!bNeedsRebuild) return;

	PlayerStatusBox->ClearChildren();
	StatusWidgets.Reset();

	if (!PlayerStatusWidgetClass) return;

	for (int32 Index = 0; Index < Count; ++Index)
	{
		ULoadingPlayerStatusWidget* StatusWidget = CreateWidget<ULoadingPlayerStatusWidget>(this, PlayerStatusWidgetClass);
		if (StatusWidget)
		{
			PlayerStatusBox->AddChild(StatusWidget);
			StatusWidgets.Add(StatusWidget);
		}
	}
}

void ULoadingScreenWidget::UpdateLoadingProgress(
	float TargetProgress,
	bool bLoadingComplete,
	float InFinishDurationSeconds,
	float InWarmupDurationSeconds,
	float InWarmupMax,
	float InMainMax
)
{
	PendingTargetProgress = TargetProgress;
	bPendingLoadingComplete = bLoadingComplete;
	bHasPendingProgress = true;

	this->FinishDurationSeconds = InFinishDurationSeconds;
	this->WarmupDurationSeconds = InWarmupDurationSeconds;
	this->WarmupMax = InWarmupMax;
	this->MainMax = InMainMax;

}

void ULoadingScreenWidget::ApplyProgressFromTarget(float TargetProgress, bool bLoadingComplete)
{
	const double Now = FPlatformTime::Seconds();
	float DeltaSeconds = static_cast<float>(FMath::Max(0.0, Now - LastUpdateSeconds));
	LastUpdateSeconds = Now;
	DeltaSeconds = FMath::Min(DeltaSeconds, 0.1f);

	const float WarmupAlpha = WarmupDurationSeconds > 0.0f
		? FMath::Clamp(static_cast<float>((Now - FirstSeenSeconds) / WarmupDurationSeconds), 0.0f, 1.0f)
		: 1.0f;
	const float WarmupProgress = WarmupAlpha * WarmupMax;

	float TargetDisplay = WarmupProgress;
	if (TargetProgress > 0.0f)
	{
		const float MainProgress = WarmupMax + (FMath::Clamp(TargetProgress, 0.0f, 1.0f) * (MainMax - WarmupMax));
		TargetDisplay = FMath::Max(TargetDisplay, MainProgress);
	}

	if (bLoadingComplete && !bFinishPhase)
	{
		bFinishPhase = true;
		FinishStartSeconds = Now;
	}

	if (bFinishPhase)
	{
		const float FinishAlpha = FinishDurationSeconds > 0.0f
			? FMath::Clamp(static_cast<float>((Now - FinishStartSeconds) / FinishDurationSeconds), 0.0f, 1.0f)
			: 1.0f;
		TargetDisplay = FMath::Lerp(TargetDisplay, 1.0f, FinishAlpha);
	}

	DisplayProgress = FMath::Clamp(TargetDisplay, 0.0f, 1.0f);

	if (LoadingProgressBar)
	{
		LoadingProgressBar->SetPercent(DisplayProgress);
	}
	if (LoadingPercentText)
	{
		const int32 Percent = FMath::RoundToInt(DisplayProgress * 100.0f);
		LoadingPercentText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), Percent)));
	}
}

