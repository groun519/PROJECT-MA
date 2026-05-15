#include "Widget/Skill/MASkillIconWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
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
		static const FName IconTextureParameterName(TEXT("IconTexture"));
		static const FName SubIconTextureParameterName(TEXT("SubIconTexture"));
		static const FName IconColorParameterName(TEXT("IconColor"));
		static const FName InnerColorParameterName(TEXT("InnerColor"));
		static const FName UseIconParameterName(TEXT("UseIcon"));
		static const FName UseSubIconParameterName(TEXT("UseSubIcon"));

		if (IconData.Icon)
		{
			IconMaterial->SetTextureParameterValue(IconTextureParameterName, IconData.Icon);
		}
		else
		{
			IconMaterial->SetTextureParameterValue(IconTextureParameterName, nullptr);
		}
		if (AssembledSubIcon)
		{
			IconMaterial->SetTextureParameterValue(SubIconTextureParameterName, AssembledSubIcon);
		}
		else
		{
			IconMaterial->SetTextureParameterValue(SubIconTextureParameterName, nullptr);
		}
		IconMaterial->SetVectorParameterValue(IconColorParameterName, IconData.IconColor);
		IconMaterial->SetVectorParameterValue(InnerColorParameterName, IconData.InnerColor);
		IconMaterial->SetScalarParameterValue(UseIconParameterName, IconData.Icon ? 1.f : 0.f);
		IconMaterial->SetScalarParameterValue(UseSubIconParameterName, AssembledSubIcon ? 1.f : 0.f);
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
