#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MAStatusEffectDurationWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class P_MA_API UMAStatusEffectDurationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void SetStatusEffectDuration(const FText& InLabel, float InDuration, float InRemainingDuration);
	void ClearStatusEffectDuration();

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> LabelText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> DurationProgressBar;

private:
	float DurationSeconds = 0.f;
	float EndTimeSeconds = 0.f;
	bool bHasActiveStatusEffectDuration = false;
};
