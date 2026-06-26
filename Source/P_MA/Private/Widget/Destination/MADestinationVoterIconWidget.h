#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MADestinationVoterIconWidget.generated.h"

class UImage;

UCLASS()
class P_MA_API UMADestinationVoterIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetVoterColors(const FLinearColor& BodyColor, const FLinearColor& EyeColor);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> BodyColorImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> EyeColorImage;
};
