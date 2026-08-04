#include "Widget/MADescriptionTooltipWidget.h"

#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"

void UMADescriptionTooltipWidget::SetDescription(const FText& InTitle, const FText& InDescription)
{
	if (TitleText)
	{
		TitleText->SetText(InTitle);
		TitleText->SetVisibility(InTitle.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	}

	if (DescriptionText)
	{
		DescriptionText->SetText(InDescription);
		DescriptionText->SetVisibility(InDescription.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	}
}
