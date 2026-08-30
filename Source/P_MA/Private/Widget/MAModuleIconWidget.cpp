#include "Widget/MAModuleIconWidget.h"

#include "Display/MADisplayTypes.h"
#include "MAMaterialParams.h"
#include "Materials/MaterialInstanceDynamic.h"

void UMAModuleIconWidget::SetIconData(const FMAIconData& IconData)
{
	if (UMaterialInstanceDynamic* IconMaterial = GetDynamicMaterial())
	{
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_IconTexture, IconData.Icon);
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_SubIconTexture, IconData.SubIcon);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_FrameColor, IconData.FrameColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_IconColor, IconData.IconColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_InnerColor, IconData.InnerColor);
		IconMaterial->SetScalarParameterValue(PARAM_ModuleIcon_UseIcon, IconData.Icon ? 1.f : 0.f);
		IconMaterial->SetScalarParameterValue(PARAM_ModuleIcon_UseSubIcon, IconData.SubIcon ? 1.f : 0.f);
	}
	else if (IconData.Icon)
	{
		SetBrushFromTexture(IconData.Icon);
	}
	else
	{
		SetBrush(FSlateBrush());
	}
}
