#pragma once

#include "CoreMinimal.h"
#include "Widget/MADescriptionTooltipWidget.h"
#include "MADisplayTooltipWidget.generated.h"

class UImage;
class UPanelWidget;
class UMASkillTagBadgeWidget;
class UMASkillTooltipMessageWidget;
enum class EMASkillTooltipTextType : uint8;
struct FMADisplayData;
struct FMAIconData;

UCLASS()
class P_MA_API UMADisplayTooltipWidget : public UMADescriptionTooltipWidget
{
	GENERATED_BODY()

public:
	virtual void SetDisplayData(const FMADisplayData& DisplayData);
	void SetBadge(const FText& Text, const FLinearColor& BackgroundColor);
	void SetMessage(const FText& Message);

protected:
	void AddMessage(const FText& Message, EMASkillTooltipTextType TextType);

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> SkillIconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> MessagePanel;

	UPROPERTY(EditDefaultsOnly, Category="Tooltip")
	TSubclassOf<UMASkillTooltipMessageWidget> MessageWidgetClass;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> TagBadgePanel;

	UPROPERTY(EditDefaultsOnly, Category="Tooltip")
	TSubclassOf<UMASkillTagBadgeWidget> TagBadgeWidgetClass;

private:
	void SetIconData(const FMAIconData& IconData);
};
