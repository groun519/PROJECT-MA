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
class UMASkillModule;
class UMASkillTagBadgeWidget;
class UMASkillTooltipMessageWidget;
struct FMASkillIconData;

UCLASS()
class P_MA_API UMASkillTooltipWidget : public UMADescriptionTooltipWidget
{
	GENERATED_BODY()

public:
	void SetSkillTooltip(
		const UMASkillModule* SkillModule,
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
	void SetIconData(const FMASkillIconData& IconData, UTexture2D* AssembledSubIcon, const FLinearColor& FrameColor);
	void SetCooldown(const UMASkillModule* SkillModule);
	void SetTooltipTags(const FGameplayTagContainer& TooltipTags, const UDataTable* WarningTextDataTable);
	void SetTooltipMessages(
		const FGameplayTagContainer& TooltipTags,
		const FGameplayTag& InactiveReasonTag,
		const UDataTable* WarningTextDataTable);
	FText ResolveCooldownText(const UMASkillModule* SkillModule) const;
};
