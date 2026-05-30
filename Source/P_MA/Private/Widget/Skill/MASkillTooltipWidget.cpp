#include "Widget/Skill/MASkillTooltipWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "MAMaterialParams.h"
#include "Materials/MaterialInstanceDynamic.h"

void UMASkillTooltipWidget::SetSkillTooltip(
	const UMASkillDefinition* SkillDefinition,
	const FText& InCooldownText,
	const FText& InWarningText)
{
	const FMASkillDefinitionDisplayData DisplayData = SkillDefinition
		? SkillDefinition->GetDisplayData()
		: FMASkillDefinitionDisplayData();

	SetDescription(DisplayData.DisplayName, DisplayData.Description);
	SetIconData(DisplayData.IconData, SkillDefinition ? SkillDefinition->GetAssembledSubIcon() : nullptr);
	SetCooldownText(InCooldownText);
	SetWarningText(InWarningText);
}

void UMASkillTooltipWidget::SetIconData(const FMASkillDefinitionIconData& IconData, UTexture2D* AssembledSubIcon)
{
	if (!SkillIconImage) return;

	if (UMaterialInstanceDynamic* IconMaterial = SkillIconImage->GetDynamicMaterial())
	{
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_IconTexture, IconData.Icon);
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_SubIconTexture, AssembledSubIcon);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_IconColor, IconData.IconColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_InnerColor, IconData.InnerColor);
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

void UMASkillTooltipWidget::SetCooldownText(const FText& InCooldownText)
{
	const ESlateVisibility CooldownVisibility = InCooldownText.IsEmpty()
		? ESlateVisibility::Collapsed
		: ESlateVisibility::SelfHitTestInvisible;

	if (CooldownText)
	{
		CooldownText->SetText(InCooldownText);
		CooldownText->SetVisibility(CooldownVisibility);
	}
	if (CooldownIconImage)
	{
		CooldownIconImage->SetVisibility(CooldownVisibility);
	}
}

void UMASkillTooltipWidget::SetWarningText(const FText& InWarningText)
{
	const ESlateVisibility WarningVisibility = InWarningText.IsEmpty()
		? ESlateVisibility::Collapsed
		: ESlateVisibility::SelfHitTestInvisible;

	WarningText->SetText(InWarningText);
	WarningText->SetVisibility(WarningVisibility);
	WarningIconImage->SetVisibility(WarningVisibility);
}

