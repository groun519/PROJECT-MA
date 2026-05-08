#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MADescriptionTooltipWidget.generated.h"

class UTextBlock;
class URichTextBlock;

UCLASS()
class P_MA_API UMADescriptionTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetDescription(const FText& InTitle, const FText& InDescription);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<URichTextBlock> DescriptionText;
};
