#include "Widget/Skill/MASkillModuleDragVisualWidget.h"

#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UMASkillModuleDragVisualWidget::SetIcon(UTexture2D* InIconTexture, FLinearColor InIconColor)
{
	if (!DragIconImage) return;

	if (InIconTexture)
	{
		if (UMaterialInstanceDynamic* IconMaterial = DragIconImage->GetDynamicMaterial())
		{
			static const FName IconTextureParameterName(TEXT("IconTexture"));
			static const FName IconColorParameterName(TEXT("IconColor"));
			IconMaterial->SetTextureParameterValue(IconTextureParameterName, InIconTexture);
			IconMaterial->SetVectorParameterValue(IconColorParameterName, InIconColor);
			DragIconImage->SetVisibility(ESlateVisibility::Visible);
			return;
		}
	}

	DragIconImage->SetVisibility(ESlateVisibility::Collapsed);
}
