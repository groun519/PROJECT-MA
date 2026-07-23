#include "Widget/Skill/MASkillTooltipWidget.h"

#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "GAS/Skill/Definition/MASkillWarningTextData.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "MAMaterialParams.h"
#include "Materials/MaterialInstanceDynamic.h"
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
	const FMASkillDefinitionDisplayData DisplayData = SkillModule
		? SkillModule->GetDisplayData()
		: FMASkillDefinitionDisplayData();
	const UMAModuleQualityData* ModuleQualityData = UMAGameSettings::Get()->GetModuleQualityData();
	const FMASkillIconData IconData = SkillModule
		? SkillModule->ResolveIconData(ModuleQualityData)
		: FMASkillIconData();

	SetDescription(DisplayData.DisplayName, DisplayData.Description);
	SetIconData(
		IconData,
		SkillModule ? SkillModule->GetAssembledSubIcon() : nullptr,
		SkillModule ? SkillModule->ResolveFrameColor(ModuleQualityData) : FLinearColor::White);
	SetCooldown(SkillModule);
	const FGameplayTagContainer TooltipTags = SkillModule
		? SkillModule->GetTooltipTags()
		: FGameplayTagContainer();
	SetTooltipTags(bShowTagsAndMessages ? TooltipTags : FGameplayTagContainer(), WarningTextDataTable);
	SetTooltipMessages(
		bShowTagsAndMessages ? TooltipTags : FGameplayTagContainer(),
		bShowTagsAndMessages ? InactiveReasonTag : FGameplayTag(),
		WarningTextDataTable);

	SubModulePanel->ClearChildren();
	SubModulePanel->SetVisibility(ESlateVisibility::Collapsed);
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

void UMASkillTooltipWidget::SetIconData(const FMASkillIconData& IconData, UTexture2D* AssembledSubIcon, const FLinearColor& FrameColor)
{
	if (!SkillIconImage) return;

	if (UMaterialInstanceDynamic* IconMaterial = SkillIconImage->GetDynamicMaterial())
	{
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_IconTexture, IconData.Icon);
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_SubIconTexture, AssembledSubIcon);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_IconColor, IconData.IconColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_InnerColor, IconData.InnerColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_FrameColor, FrameColor);
		IconMaterial->SetScalarParameterValue(PARAM_ModuleIcon_UseIcon, IconData.Icon ? 1.f : 0.f);
		IconMaterial->SetScalarParameterValue(PARAM_ModuleIcon_UseSubIcon, AssembledSubIcon ? 1.f : 0.f);
	}
	else if (IconData.Icon)
	{
		SkillIconImage->SetBrushFromTexture(IconData.Icon);
	}
	else
	{
		SkillIconImage->SetBrush(FSlateBrush());
	}

	SkillIconImage->SetVisibility(IconData.Icon || AssembledSubIcon ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
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

		UMASkillTooltipMessageWidget* MessageWidget =
			CreateWidget<UMASkillTooltipMessageWidget>(this, MessageWidgetClass);
		if (!MessageWidget) continue;

		MessageWidget->SetMessage(TextRow->WarningText, TextRow->TextType);
		MessagePanel->AddChild(MessageWidget);
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
