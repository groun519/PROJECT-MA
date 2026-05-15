#include "Widget/Skill/MASkillTooltipWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "Materials/MaterialInstanceDynamic.h"

void UMASkillTooltipWidget::SetSkillTooltip(const UMASkillDefinition* SkillDefinition, const FText& InCooldownText)
{
	const FMASkillDefinitionDisplayData DisplayData = SkillDefinition
		? SkillDefinition->GetDisplayData()
		: FMASkillDefinitionDisplayData();

	SetDescription(DisplayData.DisplayName, DisplayData.Description);
	SetIconData(DisplayData.IconData, SkillDefinition ? SkillDefinition->GetAssembledSubIcon() : nullptr);
	SetCooldownText(InCooldownText);
}

void UMASkillTooltipWidget::SetIconData(const FMASkillDefinitionIconData& IconData, UTexture2D* AssembledSubIcon)
{
	if (!SkillIconImage) return;

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
