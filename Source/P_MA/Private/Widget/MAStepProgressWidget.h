#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MAStepProgressWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class P_MA_API UMAStepProgressWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	void SetStepProgress(const FText& InLabel, float InDuration, float InRemainingDuration);
	void ClearStepProgress();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LabelText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> DurationProgressBar;

private:
	void RefreshFromOwner();
};
