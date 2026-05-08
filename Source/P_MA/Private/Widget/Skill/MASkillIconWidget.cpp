#include "Widget/Skill/MASkillIconWidget.h"

#include "Components/TextBlock.h"

void UMASkillIconWidget::SetHotkeyText(const FText& InText)
{
	if (!HotkeyText) return;

	HotkeyText->SetText(InText);
	HotkeyText->SetVisibility(InText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
}
