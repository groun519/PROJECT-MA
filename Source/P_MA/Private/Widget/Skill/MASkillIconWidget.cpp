#include "Widget/Skill/MASkillIconWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "MAMaterialParams.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Widget/Skill/MASkillTooltipWidget.h"

void UMASkillIconWidget::SetHotkeyText(const FText& InText)
{
	if (!HotkeyText) return;

	HotkeyText->SetText(InText);
	HotkeyText->SetVisibility(InText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
}

void UMASkillIconWidget::SetSkillDefinition(const UMASkillDefinition* SkillDefinition, FText InCooldownText)
{
	if (!SkillIconImage) return;

	const FMASkillDefinitionIconData IconData = SkillDefinition
		? SkillDefinition->GetDisplayData().IconData
		: FMASkillDefinitionIconData();
	UTexture2D* AssembledSubIcon = SkillDefinition ? SkillDefinition->GetAssembledSubIcon() : nullptr;

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

	SkillIconImage->SetVisibility(ESlateVisibility::Visible);
	RefreshTooltip(SkillDefinition, InCooldownText);
}

void UMASkillIconWidget::RefreshTooltip(const UMASkillDefinition* SkillDefinition, const FText& InCooldownText)
{
	if (!SkillDefinition || !TooltipWidgetClass)
	{
		SetToolTip(nullptr);
		return;
	}

	UMASkillTooltipWidget* TooltipWidget = CreateWidget<UMASkillTooltipWidget>(GetOwningPlayer(), TooltipWidgetClass);
	if (!TooltipWidget)
	{
		SetToolTip(nullptr);
		return;
	}

	TooltipWidget->SetSkillTooltip(SkillDefinition, InCooldownText);
	SetToolTip(TooltipWidget);
}

