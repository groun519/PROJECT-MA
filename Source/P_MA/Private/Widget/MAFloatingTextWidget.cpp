#include "Widget/MAFloatingTextWidget.h"

#include "Components/TextBlock.h"

void UMAFloatingTextWidget::SetDisplayText(const FText& Text, const FLinearColor& Color, const FLinearColor& OutlineColor)
{
	if (DamageText)
	{
		DamageText->SetText(Text);
		DamageText->SetColorAndOpacity(FSlateColor(Color));

		FSlateFontInfo FontInfo = DamageText->GetFont();
		FontInfo.OutlineSettings.OutlineSize = OutlineColor.A > 0.f ? 1 : 0;
		FontInfo.OutlineSettings.OutlineColor = OutlineColor;
		DamageText->SetFont(FontInfo);
	}

	if (FadeUpAnim)
	{
		PlayAnimation(FadeUpAnim);
	}
}
