#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MAFlowProgressWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class P_MA_API UMAFlowProgressWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	void SetFlowProgress(const FText& InLabel, float InDuration, float InRemainingDuration);
	void ClearFlowProgress();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LabelText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> DurationProgressBar;

private:
	void RefreshFromOwner();
};
