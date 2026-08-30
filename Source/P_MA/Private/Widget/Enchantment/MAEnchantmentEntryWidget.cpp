#include "Widget/Enchantment/MAEnchantmentEntryWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "Setting/MAGameSettings.h"
#include "Widget/MAModuleIconWidget.h"
#include "Widget/Skill/MASkillTooltipWidget.h"

void UMAEnchantmentEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EntryButton->OnClicked.RemoveDynamic(this, &UMAEnchantmentEntryWidget::HandleEntryButtonClicked);
	EntryButton->OnClicked.AddDynamic(this, &UMAEnchantmentEntryWidget::HandleEntryButtonClicked);
}

void UMAEnchantmentEntryWidget::SetModule(const UMASkillModule& Module, const int32 Count)
{
	SetModuleVisual(
		Module,
		FText::Format(
			NSLOCTEXT("MAEnchantmentEntryWidget", "CountFormat", "x{0}"),
			FText::AsNumber(Count)));

	if (!TooltipWidgetClass) return;
	UMASkillTooltipWidget* TooltipWidget = CreateWidget<UMASkillTooltipWidget>(GetOwningPlayer(), TooltipWidgetClass);
	if (!TooltipWidget) return;

	TooltipWidget->SetSkillTooltip(
		&Module,
		FGameplayTag(),
		UMAGameSettings::Get()->GetWarningTextDataTable());
	SetToolTip(TooltipWidget);
}

void UMAEnchantmentEntryWidget::SetModuleInstance(
	const UMASkillModuleInstance& ModuleInstance,
	const FText& PositionText)
{
	const UMASkillModule* Module = ModuleInstance.GetRootModule();
	check(Module);
	SetModuleVisual(*Module, PositionText);

	if (!TooltipWidgetClass) return;
	UMASkillTooltipWidget* TooltipWidget = CreateWidget<UMASkillTooltipWidget>(GetOwningPlayer(), TooltipWidgetClass);
	if (!TooltipWidget) return;

	TooltipWidget->SetModuleTooltip(
		ModuleInstance,
		UMAGameSettings::Get()->GetWarningTextDataTable());
	SetToolTip(TooltipWidget);
}

void UMAEnchantmentEntryWidget::SetSelected(const bool bSelected)
{
	SelectionImage->SetVisibility(bSelected
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
}

void UMAEnchantmentEntryWidget::SetModuleVisual(
	const UMASkillModule& Module,
	const FText& DetailText)
{
	const FMADisplayData DisplayData = Module.ResolveDisplayData(
		UMAGameSettings::Get()->GetModuleQualityData());
	const FMAIconData& IconData = DisplayData.IconData;
	ModuleIconImage->SetIconData(IconData);

	NameText->SetText(DisplayData.DisplayName);
	CountText->SetText(DetailText);
	CountText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	SelectionImage->SetColorAndOpacity(IconData.FrameColor);
}

void UMAEnchantmentEntryWidget::HandleEntryButtonClicked()
{
	OnSelected.Broadcast();
}
