#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Widget/MADescriptionTooltipWidget.h"
#include "MASkillTooltipWidget.generated.h"

class UImage;
class UPanelWidget;
class UTextBlock;
class UTexture2D;
class UDataTable;
class UMASkillDefinition;
class UMASkillTagBadgeWidget;
class UMASkillTooltipMessageWidget;
struct FMASkillDefinitionIconData;

UCLASS()
class P_MA_API UMASkillTooltipWidget : public UMADescriptionTooltipWidget
{
	GENERATED_BODY()

public:
	void SetSkillTooltip(
		const UMASkillDefinition* SkillDefinition,
		const FGameplayTag& InactiveReasonTag = FGameplayTag(),
		const UDataTable* WarningTextDataTable = nullptr,
		bool bShowTagsAndMessages = true);

protected:
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> SkillIconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CooldownText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> CooldownIconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> MessagePanel;

	UPROPERTY(EditDefaultsOnly, Category="Tooltip")
	TSubclassOf<UMASkillTooltipMessageWidget> MessageWidgetClass;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> TagBadgePanel;

	UPROPERTY(EditDefaultsOnly, Category="Tooltip")
	TSubclassOf<UMASkillTagBadgeWidget> TagBadgeWidgetClass;

private:
	void SetIconData(const FMASkillDefinitionIconData& IconData, UTexture2D* AssembledSubIcon, const FLinearColor& FrameColor);
	void SetCooldown(const UMASkillDefinition* SkillDefinition);
	void SetTooltipTags(const FGameplayTagContainer& TooltipTags, const UDataTable* WarningTextDataTable);
	void SetTooltipMessages(
		const FGameplayTagContainer& TooltipTags,
		const FGameplayTag& InactiveReasonTag,
		const UDataTable* WarningTextDataTable);
	FText ResolveCooldownText(const UMASkillDefinition* SkillDefinition) const;
};
