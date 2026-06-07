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

	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	FLinearColor PositiveCooldownColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	FLinearColor NegativeCooldownColor = FLinearColor(0.25f, 0.75f, 1.f, 1.f);

private:
	void SetIconData(const FMASkillDefinitionIconData& IconData, UTexture2D* AssembledSubIcon);
	void SetCooldown(const UMASkillDefinition* SkillDefinition);
	void SetWarningText(const FText& InWarningText);
	FText ResolveCooldownText(const UMASkillDefinition* SkillDefinition) const;
};
