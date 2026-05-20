#include "Widget/MAFloatingTextWidget.h"

#include "Components/TextBlock.h"

void UMAFloatingTextWidget::SetDisplayText(const FText& Text, const FLinearColor& Color)
{
	if (DamageText)
	{
		DamageText->SetText(Text);
		DamageText->SetColorAndOpacity(FSlateColor(Color));
	}

	if (FadeUpAnim)
	{
		PlayAnimation(FadeUpAnim);
	}
}
