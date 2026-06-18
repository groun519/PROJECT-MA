#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/Skill/Definition/MASkillWarningTextData.h"
#include "MASkillTooltipMessageWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class P_MA_API UMASkillTooltipMessageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetMessage(const FText& Message, EMASkillTooltipTextType TextType);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> MessageText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> WarningIconImage;

	UPROPERTY(EditDefaultsOnly, Category="Message")
	FLinearColor NormalTextColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category="Message")
	FLinearColor WarningTextColor = FLinearColor(1.f, 0.05f, 0.f, 1.f);
};
