#include "Widget/MADisplayTooltipWidget.h"

#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Display/MADisplayTypes.h"
#include "GAS/Skill/Definition/MASkillWarningTextData.h"
#include "MAMaterialParams.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Widget/Skill/MASkillTagBadgeWidget.h"
#include "Widget/Skill/MASkillTooltipMessageWidget.h"

void UMADisplayTooltipWidget::SetDisplayData(const FMADisplayData& DisplayData)
{
	SetDescription(DisplayData.DisplayName, DisplayData.Description);
	SetIconData(DisplayData.IconData);

	TagBadgePanel->ClearChildren();
	TagBadgePanel->SetVisibility(ESlateVisibility::Collapsed);
	MessagePanel->ClearChildren();
	MessagePanel->SetVisibility(ESlateVisibility::Collapsed);
}

void UMADisplayTooltipWidget::SetBadge(
	const FText& Text,
	const FLinearColor& BackgroundColor)
{
	TagBadgePanel->ClearChildren();
	if (TagBadgeWidgetClass && !Text.IsEmpty())
	{
		UMASkillTagBadgeWidget* BadgeWidget =
			CreateWidget<UMASkillTagBadgeWidget>(this, TagBadgeWidgetClass);
		if (BadgeWidget)
		{
			FMASkillTagStyle Style;
			Style.BackgroundColor = BackgroundColor;
			BadgeWidget->SetTag(Text, Style);
			TagBadgePanel->AddChild(BadgeWidget);
		}
	}
	TagBadgePanel->SetVisibility(TagBadgePanel->GetChildrenCount() > 0
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
}

void UMADisplayTooltipWidget::SetMessage(const FText& Message)
{
	MessagePanel->ClearChildren();
	AddMessage(Message, EMASkillTooltipTextType::Normal);
	MessagePanel->SetVisibility(MessagePanel->GetChildrenCount() > 0
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
}

void UMADisplayTooltipWidget::AddMessage(
	const FText& Message,
	const EMASkillTooltipTextType TextType)
{
	if (!MessageWidgetClass || Message.IsEmpty()) return;

	UMASkillTooltipMessageWidget* MessageWidget =
		CreateWidget<UMASkillTooltipMessageWidget>(this, MessageWidgetClass);
	if (!MessageWidget) return;

	MessageWidget->SetMessage(Message, TextType);
	MessagePanel->AddChild(MessageWidget);
}

void UMADisplayTooltipWidget::SetIconData(const FMAIconData& IconData)
{
	if (!SkillIconImage) return;

	if (UMaterialInstanceDynamic* IconMaterial = SkillIconImage->GetDynamicMaterial())
	{
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_IconTexture, IconData.Icon);
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_SubIconTexture, IconData.SubIcon);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_IconColor, IconData.IconColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_InnerColor, IconData.InnerColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_FrameColor, IconData.FrameColor);
		IconMaterial->SetScalarParameterValue(PARAM_ModuleIcon_UseIcon, IconData.Icon ? 1.f : 0.f);
		IconMaterial->SetScalarParameterValue(PARAM_ModuleIcon_UseSubIcon, IconData.SubIcon ? 1.f : 0.f);
	}
	else if (IconData.Icon)
	{
		SkillIconImage->SetBrushFromTexture(IconData.Icon);
	}
	else
	{
		SkillIconImage->SetBrush(FSlateBrush());
	}

	SkillIconImage->SetVisibility(IconData.Icon || IconData.SubIcon
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
}
