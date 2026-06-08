#include "Widget/Skill/MASkillTooltipWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "MAMaterialParams.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Setting/MAGameSettings.h"

void UMASkillTooltipWidget::SetSkillTooltip(
	const UMASkillDefinition* SkillDefinition,
	const FText& InWarningText)
{
	const FMASkillDefinitionDisplayData DisplayData = SkillDefinition
		? SkillDefinition->GetDisplayData()
		: FMASkillDefinitionDisplayData();
	const UMAModuleQualityData* ModuleQualityData = UMAGameSettings::Get()->GetModuleQualityData();
	const FMASkillDefinitionIconData IconData = SkillDefinition
		? SkillDefinition->ResolveIconData(ModuleQualityData)
		: DisplayData.IconData;

	SetDescription(DisplayData.DisplayName, DisplayData.Description);
	SetIconData(
		IconData,
		SkillDefinition ? SkillDefinition->GetAssembledSubIcon() : nullptr,
		SkillDefinition ? SkillDefinition->ResolveFrameColor(ModuleQualityData) : FLinearColor::White);
	SetCooldown(SkillDefinition);
	SetWarningText(InWarningText);
}

void UMASkillTooltipWidget::SetIconData(const FMASkillDefinitionIconData& IconData, UTexture2D* AssembledSubIcon, const FLinearColor& FrameColor)
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

void UMASkillTooltipWidget::SetCooldown(const UMASkillDefinition* SkillDefinition)
{
	const FText CooldownTextValue = ResolveCooldownText(SkillDefinition);
	const ESlateVisibility CooldownVisibility = CooldownTextValue.IsEmpty()
		? ESlateVisibility::Collapsed
		: ESlateVisibility::SelfHitTestInvisible;
	const UMAGameSettings* GameSettings = UMAGameSettings::Get();
	const FLinearColor CooldownColor = SkillDefinition && SkillDefinition->GetCooldownSeconds() < 0.f
		? GameSettings->NegativeCooldownColor
		: GameSettings->PositiveCooldownColor;

	if (CooldownText)
	{
		CooldownText->SetText(CooldownTextValue);
		if (SkillDefinition)
		{
			CooldownText->SetColorAndOpacity(FSlateColor(CooldownColor));
		}
		CooldownText->SetVisibility(CooldownVisibility);
	}
	if (CooldownIconImage)
	{
		if (SkillDefinition)
		{
			CooldownIconImage->SetColorAndOpacity(CooldownColor);
		}
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

FText UMASkillTooltipWidget::ResolveCooldownText(const UMASkillDefinition* SkillDefinition) const
{
	if (!SkillDefinition || FMath::IsNearlyZero(SkillDefinition->GetCooldownSeconds())) return FText();

	FNumberFormattingOptions FormattingOptions;
	FormattingOptions.MinimumFractionalDigits = 0;
	FormattingOptions.MaximumFractionalDigits = FMath::Abs(SkillDefinition->GetCooldownSeconds()) >= 1.f ? 1 : 2;

	return FText::Format(
		NSLOCTEXT("MASkillTooltipWidget", "CooldownSecondsFormat", "{0}s"),
		FText::AsNumber(SkillDefinition->GetCooldownSeconds(), &FormattingOptions));
}
