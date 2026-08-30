#include "Widget/Enchantment/MAEnchantmentNodeWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "Setting/MAGameSettings.h"
#include "Widget/MAModuleIconWidget.h"
#include "Widget/Skill/MASkillTooltipWidget.h"

void UMAEnchantmentNodeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	NodeButton->OnClicked.RemoveDynamic(this, &UMAEnchantmentNodeWidget::HandleNodeButtonClicked);
	NodeButton->OnClicked.AddDynamic(this, &UMAEnchantmentNodeWidget::HandleNodeButtonClicked);
}

void UMAEnchantmentNodeWidget::SetModule(const UMASkillModule& Module)
{
	SetModuleVisual(Module);
	if (!TooltipWidgetClass) return;

	UMASkillTooltipWidget* TooltipWidget = CreateWidget<UMASkillTooltipWidget>(
		GetOwningPlayer(),
		TooltipWidgetClass);
	if (!TooltipWidget) return;

	TooltipWidget->SetSkillTooltip(
		&Module,
		FGameplayTag(),
		UMAGameSettings::Get()->GetWarningTextDataTable());
	SetToolTip(TooltipWidget);
}

void UMAEnchantmentNodeWidget::SetModuleInstance(const UMASkillModuleInstance& ModuleInstance)
{
	const UMASkillModule* Module = ModuleInstance.GetRootModule();
	check(Module);
	SetModuleVisual(*Module);
	if (!TooltipWidgetClass) return;

	UMASkillTooltipWidget* TooltipWidget = CreateWidget<UMASkillTooltipWidget>(
		GetOwningPlayer(),
		TooltipWidgetClass);
	if (!TooltipWidget) return;

	TooltipWidget->SetModuleTooltip(
		ModuleInstance,
		UMAGameSettings::Get()->GetWarningTextDataTable());
	SetToolTip(TooltipWidget);
}

void UMAEnchantmentNodeWidget::SetEmpty()
{
	ModuleIconImage->SetVisibility(ESlateVisibility::Collapsed);
	EmptyText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	SetToolTip(nullptr);
}

void UMAEnchantmentNodeWidget::SetSelected(const bool bSelected)
{
	SelectionImage->SetVisibility(bSelected
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
}

void UMAEnchantmentNodeWidget::SetModuleVisual(const UMASkillModule& Module)
{
	ModuleIconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	const FMADisplayData DisplayData = Module.ResolveDisplayData(
		UMAGameSettings::Get()->GetModuleQualityData());
	const FMAIconData& IconData = DisplayData.IconData;
	ModuleIconImage->SetIconData(IconData);

	SelectionImage->SetColorAndOpacity(IconData.FrameColor);
	EmptyText->SetVisibility(ESlateVisibility::Collapsed);
}

void UMAEnchantmentNodeWidget::HandleNodeButtonClicked()
{
	OnSelected.Broadcast();
}
