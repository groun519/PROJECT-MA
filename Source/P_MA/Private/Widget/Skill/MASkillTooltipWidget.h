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
	void SetSkillTooltip(
		const UMASkillDefinition* SkillDefinition,
		const FText& InWarningText = FText());

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> SkillIconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CooldownText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> CooldownIconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> WarningText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> WarningIconImage;

private:
	void SetIconData(const FMASkillDefinitionIconData& IconData, UTexture2D* AssembledSubIcon, const FLinearColor& FrameColor);
	void SetCooldown(const UMASkillDefinition* SkillDefinition);
	void SetWarningText(const FText& InWarningText);
	FText ResolveCooldownText(const UMASkillDefinition* SkillDefinition) const;
};
