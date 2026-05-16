#include "Widget/Skill/MASkillModuleDragVisualWidget.h"

#include "Components/Image.h"
#include "MAMaterialParams.h"
#include "Materials/MaterialInstanceDynamic.h"

void UMASkillModuleDragVisualWidget::SetIcon(UTexture2D* InIconTexture, FLinearColor InIconColor)
{
	if (!DragIconImage) return;

	if (InIconTexture)
	{
		if (UMaterialInstanceDynamic* IconMaterial = DragIconImage->GetDynamicMaterial())
		{
			IconMaterial->SetTextureParameterValue(PARAM_DragDropIcon_IconTexture, InIconTexture);
			IconMaterial->SetVectorParameterValue(PARAM_DragDropIcon_IconColor, InIconColor);
			DragIconImage->SetVisibility(ESlateVisibility::Visible);
			return;
		}
	}

	DragIconImage->SetVisibility(ESlateVisibility::Collapsed);
}

