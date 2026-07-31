#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Widget/MADisplayTooltipWidget.h"
#include "MASkillTooltipWidget.generated.h"

class UImage;
class UPanelWidget;
class UTextBlock;
class UDataTable;
class UMASkillModule;
class UMASkillModuleInstance;
class UMASkillSubModuleTooltipWidget;
struct FMADisplayData;

UCLASS()
class P_MA_API UMASkillTooltipWidget : public UMADisplayTooltipWidget
{
	GENERATED_BODY()

public:
	virtual void SetDisplayData(const FMADisplayData& DisplayData) override;
	void SetSkillTooltip(
		const UMASkillModule* SkillModule,
		const FGameplayTag& InactiveReasonTag = FGameplayTag(),
		const UDataTable* WarningTextDataTable = nullptr,
		bool bShowTagsAndMessages = true);
	void SetModuleTooltip(
		const UMASkillModuleInstance& ModuleInstance,
		const UDataTable* WarningTextDataTable = nullptr,
		bool bShowTagsAndMessages = true);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CooldownText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> CooldownIconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> SubModulePanel;

	UPROPERTY(EditDefaultsOnly, Category="Tooltip")
	TSubclassOf<UMASkillSubModuleTooltipWidget> SubModuleWidgetClass;

private:
	void SetCooldown(const UMASkillModule* SkillModule);
	void SetTooltipTags(const FGameplayTagContainer& TooltipTags, const UDataTable* WarningTextDataTable);
	void SetTooltipMessages(
		const FGameplayTagContainer& TooltipTags,
		const FGameplayTag& InactiveReasonTag,
		const UDataTable* WarningTextDataTable);
	FText ResolveCooldownText(const UMASkillModule* SkillModule) const;
};
