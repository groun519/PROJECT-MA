#include "Widget/Destination/MADestinationVoterIconWidget.h"

#include "Components/Image.h"

void UMADestinationVoterIconWidget::SetVoterColors(const FLinearColor& BodyColor, const FLinearColor& EyeColor)
{
	BodyColorImage->SetColorAndOpacity(BodyColor);
	EyeColorImage->SetColorAndOpacity(EyeColor);
}
