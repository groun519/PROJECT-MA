// Fill out your copyright notice in the Description page of Project Settings.

#include "LoadingScreenWidget.h"
#include "Widget/Loading/LoadingPlayerStatusWidget.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void ULoadingScreenWidget::UpdateLoadingStatus(const TArray<FLoadingPlayerStatus>& Statuses)
{
	if (!PlayerStatusBox) return;

	const int32 Count = Statuses.Num();
	EnsureEntryWidgets(Count);


	if (LoadingProgressBar || LoadingPercentText)
	{
		float LocalPercent = 0.0f;
		for (const FLoadingPlayerStatus& Status : Statuses)
		{
			if (Status.PlayerName.IsEmpty()) continue;
			if (Status.bLoaded)
			{
				LocalPercent = 1.0f;
				break;
			}
		}

		if (LoadingProgressBar)
		{
			LoadingProgressBar->SetPercent(LocalPercent);
		}
		if (LoadingPercentText)
		{
			const int32 Percent = FMath::RoundToInt(LocalPercent * 100.0f);
			LoadingPercentText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), Percent)));
		}
	}

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
