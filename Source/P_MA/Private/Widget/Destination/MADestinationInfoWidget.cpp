#include "Widget/Destination/MADestinationInfoWidget.h"

#include "Components/Image.h"

void UMADestinationInfoWidget::SetEnvIcon(UTexture2D* Icon)
{
	if (Icon)
	{
		EnvIconImage->SetBrushFromTexture(Icon);
	}
	EnvIconImage->SetVisibility(Icon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
