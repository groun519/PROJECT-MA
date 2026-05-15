#pragma once

#include "CoreMinimal.h"
#include "Widget/MADescriptionTooltipWidget.h"
#include "MASkillTooltipWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;
class UMASkillDefinition;
struct FMASkillDefinitionIconData;

UCLASS()
class P_MA_API UMASkillTooltipWidget : public UMADescriptionTooltipWidget
{
	GENERATED_BODY()

public:
	void SetSkillTooltip(const UMASkillDefinition* SkillDefinition, const FText& InCooldownText);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> SkillIconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CooldownText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> CooldownIconImage;

private:
	void SetIconData(const FMASkillDefinitionIconData& IconData, UTexture2D* AssembledSubIcon);
	void SetCooldownText(const FText& InCooldownText);
};
