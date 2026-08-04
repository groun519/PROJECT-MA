#include "Widget/MAStatusEffectDurationWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UMAStatusEffectDurationWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bHasActiveStatusEffectDuration) return;

	const UWorld* World = GetWorld();
	if (!World)
	{
		ClearStatusEffectDuration();
		return;
	}

	const float RemainingDuration = FMath::Max(EndTimeSeconds - World->GetTimeSeconds(), 0.f);
	DurationProgressBar->SetPercent(DurationSeconds > 0.f ? FMath::Clamp(RemainingDuration / DurationSeconds, 0.f, 1.f) : 0.f);

	if (RemainingDuration <= 0.f)
	{
		ClearStatusEffectDuration();
	}
}

void UMAStatusEffectDurationWidget::SetStatusEffectDuration(const FText& InLabel, const float InDuration, const float InRemainingDuration)
{
	const UWorld* World = GetWorld();

	DurationSeconds = FMath::Max(InDuration, 0.f);
	EndTimeSeconds = World ? World->GetTimeSeconds() + FMath::Max(InRemainingDuration, 0.f) : 0.f;
	bHasActiveStatusEffectDuration = true;

	LabelText->SetText(InLabel);
	DurationProgressBar->SetPercent(InDuration > 0.f ? FMath::Clamp(InRemainingDuration / InDuration, 0.f, 1.f) : 0.f);
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UMAStatusEffectDurationWidget::ClearStatusEffectDuration()
{
	DurationSeconds = 0.f;
	EndTimeSeconds = 0.f;
	bHasActiveStatusEffectDuration = false;
	LabelText->SetText(FText::GetEmpty());
	DurationProgressBar->SetPercent(0.f);
	SetVisibility(ESlateVisibility::Collapsed);
}
