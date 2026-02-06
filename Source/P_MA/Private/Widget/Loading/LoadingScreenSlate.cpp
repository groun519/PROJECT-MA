// Fill out your copyright notice in the Description page of Project Settings.

#include "LoadingScreenSlate.h"
#include "Framework/MAGameInstance.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"

void SLoadingScreenRoot::Construct(const FArguments& InArgs)
{
	GameInstance = InArgs._GameInstance;
	DisplayProgress = 0.0f;
	LastUpdateSeconds = FPlatformTime::Seconds();
	FirstSeenSeconds = LastUpdateSeconds;
	bFinishPhase = false;
	FinishStartSeconds = 0.0;

	const FSlateFontInfo PercentFont = FCoreStyle::GetDefaultFontStyle("Regular", 96);
	const FSlateFontInfo SmallFont = FCoreStyle::GetDefaultFontStyle("Regular", 22);

	ChildSlot
	[
		SNew(SBorder)
		.Padding(0)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(FMargin(40.0f, 0.0f, 40.0f, 40.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						return GetPercentText();
					})
					.Font(PercentFont)
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 10.0f, 0.0f, 0.0f))
				[
					SNew(SBox)
					.WidthOverride(600.0f)
					.HeightOverride(20.0f)
					[
						SNew(SProgressBar)
						.Percent_Lambda([this]()
						{
							return GetProgress();
						})
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Loading...")))
					.Font(SmallFont)
					.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
				]
			]
		]
	];
}

TOptional<float> SLoadingScreenRoot::GetProgress()
{
	if (!GameInstance.IsValid())
	{
		return 0.0f;
	}

	const double Now = FPlatformTime::Seconds();
	float DeltaSeconds = static_cast<float>(FMath::Max(0.0, Now - LastUpdateSeconds));
	LastUpdateSeconds = Now;
	DeltaSeconds = FMath::Min(DeltaSeconds, 0.1f);

	int32 Percent = 0;
	const float Target = GameInstance->CalculateLoadingProgress(Percent);

	const float WarmupDuration = 3.0f;
	const float WarmupMax = 0.50f;
	const float MainMax = 0.95f;
	const float FinishDuration = GameInstance->GetLoadingFinishDurationSeconds();

	const float WarmupAlpha = WarmupDuration > 0.0f
		? FMath::Clamp(static_cast<float>((Now - FirstSeenSeconds) / WarmupDuration), 0.0f, 1.0f)
		: 1.0f;
	const float WarmupProgress = WarmupAlpha * WarmupMax;

	float TargetDisplay = WarmupProgress;
	if (Target > 0.0f)
	{
		const float MainProgress = WarmupMax + (FMath::Clamp(Target, 0.0f, 1.0f) * (MainMax - WarmupMax));
		TargetDisplay = FMath::Max(TargetDisplay, MainProgress);
	}

	if (Target >= 1.0f && !bFinishPhase)
	{
		bFinishPhase = true;
		FinishStartSeconds = Now;
	}

	if (bFinishPhase)
	{
		const float FinishAlpha = FinishDuration > 0.0f
			? FMath::Clamp(static_cast<float>((Now - FinishStartSeconds) / FinishDuration), 0.0f, 1.0f)
			: 1.0f;
		TargetDisplay = FMath::Lerp(TargetDisplay, 1.0f, FinishAlpha);
	}

	DisplayProgress = FMath::Min(TargetDisplay, 1.0f);
	DisplayProgress = FMath::Clamp(DisplayProgress, 0.0f, 1.0f);
	return DisplayProgress;
}

FText SLoadingScreenRoot::GetPercentText()
{
	if (!GameInstance.IsValid())
	{
		return FText::FromString(TEXT("0%"));
	}

	const float Progress = GetProgress().Get(0.0f);
	const int32 Percent = FMath::RoundToInt(Progress * 100.0f);
	return FText::FromString(FString::Printf(TEXT("%d%%"), Percent));
}
