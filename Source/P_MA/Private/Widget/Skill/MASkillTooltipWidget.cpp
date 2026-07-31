#include "Widget/Skill/MASkillTooltipWidget.h"

#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Display/MADisplayTypes.h"
#include "GAS/Skill/Definition/MASkillWarningTextData.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "Setting/MAGameSettings.h"
#include "Widget/Skill/MASkillSubModuleTooltipWidget.h"
#include "Widget/Skill/MASkillTagBadgeWidget.h"
#include "Widget/Skill/MASkillTooltipMessageWidget.h"

void UMASkillTooltipWidget::SetSkillTooltip(
	const UMASkillModule* SkillModule,
	const FGameplayTag& InactiveReasonTag,
	const UDataTable* WarningTextDataTable,
	bool bShowTagsAndMessages)
{
	const UMAModuleQualityData* ModuleQualityData = UMAGameSettings::Get()->GetModuleQualityData();
	SetDisplayData(SkillModule
		? SkillModule->ResolveDisplayData(ModuleQualityData)
		: FMADisplayData());
	SetCooldown(SkillModule);
	const FGameplayTagContainer TooltipTags = SkillModule
		? SkillModule->GetTooltipTags()
		: FGameplayTagContainer();
	SetTooltipTags(bShowTagsAndMessages ? TooltipTags : FGameplayTagContainer(), WarningTextDataTable);
	SetTooltipMessages(
		bShowTagsAndMessages ? TooltipTags : FGameplayTagContainer(),
		bShowTagsAndMessages ? InactiveReasonTag : FGameplayTag(),
		WarningTextDataTable);
}

void UMASkillTooltipWidget::SetModuleTooltip(
	const UMASkillModuleInstance& ModuleInstance,
	const UDataTable* WarningTextDataTable,
	const bool bShowTagsAndMessages)
{
	SetSkillTooltip(
		ModuleInstance.GetRootModule(),
		ModuleInstance.GetInactiveReasonTag(),
		WarningTextDataTable,
		bShowTagsAndMessages);
	if (!SubModuleWidgetClass) return;

	for (const UMASkillModule* SubModule : ModuleInstance.GetModuleGroup().SubModules)
	{
		if (!SubModule) continue;

		UMASkillSubModuleTooltipWidget* SubModuleWidget =
			CreateWidget<UMASkillSubModuleTooltipWidget>(this, SubModuleWidgetClass);
		if (!SubModuleWidget) continue;

		SubModuleWidget->SetSubModule(*SubModule);
		SubModulePanel->AddChild(SubModuleWidget);
	}
	SubModulePanel->SetVisibility(SubModulePanel->GetChildrenCount() > 0
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
}

void UMASkillTooltipWidget::SetDisplayData(const FMADisplayData& DisplayData)
{
	Super::SetDisplayData(DisplayData);
	SetCooldown(nullptr);
	SubModulePanel->ClearChildren();
	SubModulePanel->SetVisibility(ESlateVisibility::Collapsed);
}

void UMASkillTooltipWidget::SetCooldown(const UMASkillModule* SkillModule)
{
	const FText CooldownTextValue = ResolveCooldownText(SkillModule);
	const ESlateVisibility CooldownVisibility = CooldownTextValue.IsEmpty()
		? ESlateVisibility::Collapsed
		: ESlateVisibility::SelfHitTestInvisible;
	const UMAGameSettings* GameSettings = UMAGameSettings::Get();
	const FLinearColor CooldownColor = SkillModule && SkillModule->GetCooldownSeconds() < 0.f
		? GameSettings->NegativeCooldownColor
		: GameSettings->PositiveCooldownColor;

	if (CooldownText)
	{
		CooldownText->SetText(CooldownTextValue);
		if (SkillModule)
		{
			CooldownText->SetColorAndOpacity(FSlateColor(CooldownColor));
		}
		CooldownText->SetVisibility(CooldownVisibility);
	}
	if (CooldownIconImage)
	{
		if (SkillModule)
		{
			CooldownIconImage->SetColorAndOpacity(CooldownColor);
		}
		CooldownIconImage->SetVisibility(CooldownVisibility);
	}
}

void UMASkillTooltipWidget::SetTooltipTags(const FGameplayTagContainer& TooltipTags, const UDataTable* WarningTextDataTable)
{
	UMASkillTagBadgeWidget::RefreshTagBadges(this, TagBadgePanel, TagBadgeWidgetClass, TooltipTags, WarningTextDataTable);
}

void UMASkillTooltipWidget::SetTooltipMessages(
	const FGameplayTagContainer& TooltipTags,
	const FGameplayTag& InactiveReasonTag,
	const UDataTable* WarningTextDataTable)
{
	MessagePanel->ClearChildren();
	if (!MessageWidgetClass || !WarningTextDataTable)
	{
		MessagePanel->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	TArray<FMASkillWarningTextDataRow*> TextRows;
	WarningTextDataTable->GetAllRows(TEXT("SkillTooltipMessageLookup"), TextRows);
	for (const FMASkillWarningTextDataRow* TextRow : TextRows)
	{
		if (!TextRow || TextRow->WarningText.IsEmpty()) continue;

		const bool bShouldDisplay = TextRow->TextType == EMASkillTooltipTextType::Normal
			? TooltipTags.HasTagExact(TextRow->ReasonTag)
			: InactiveReasonTag.IsValid() && TextRow->ReasonTag == InactiveReasonTag;
		if (!bShouldDisplay) continue;

		AddMessage(TextRow->WarningText, TextRow->TextType);
	}

	MessagePanel->SetVisibility(MessagePanel->GetChildrenCount() > 0
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
}

FText UMASkillTooltipWidget::ResolveCooldownText(const UMASkillModule* SkillModule) const
{
	if (!SkillModule || FMath::IsNearlyZero(SkillModule->GetCooldownSeconds())) return FText();

	FNumberFormattingOptions FormattingOptions;
	FormattingOptions.MinimumFractionalDigits = 0;
	FormattingOptions.MaximumFractionalDigits = FMath::Abs(SkillModule->GetCooldownSeconds()) >= 1.f ? 1 : 2;

	return FText::Format(
		NSLOCTEXT("MASkillTooltipWidget", "CooldownSecondsFormat", "{0}s"),
		FText::AsNumber(SkillModule->GetCooldownSeconds(), &FormattingOptions));
}
