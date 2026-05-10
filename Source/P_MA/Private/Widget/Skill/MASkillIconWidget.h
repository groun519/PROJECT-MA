#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MASkillIconWidget.generated.h"

class UImage;
class UTextBlock;
class UMASkillDefinition;
class UMASkillTooltipWidget;

UCLASS()
class P_MA_API UMASkillIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHotkeyText(const FText& InText);
	void SetSkillDefinition(const UMASkillDefinition* SkillDefinition, FText InCooldownText = FText());

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> SkillIconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HotkeyText;

	UPROPERTY(EditDefaultsOnly, Category="Tooltip")
	TSubclassOf<UMASkillTooltipWidget> TooltipWidgetClass;

private:
	void RefreshTooltip(const UMASkillDefinition* SkillDefinition, const FText& InCooldownText);
};
