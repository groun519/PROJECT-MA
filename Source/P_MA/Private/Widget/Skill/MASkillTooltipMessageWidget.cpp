#include "Widget/Skill/MASkillTooltipMessageWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UMASkillTooltipMessageWidget::SetMessage(const FText& Message, EMASkillTooltipTextType TextType)
{
	const bool bWarning = TextType == EMASkillTooltipTextType::Warning;

	MessageText->SetText(Message);
	MessageText->SetColorAndOpacity(FSlateColor(bWarning ? WarningTextColor : NormalTextColor));
	WarningIconImage->SetVisibility(bWarning
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
}
