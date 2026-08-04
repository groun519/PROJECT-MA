#include "Widget/Skill/MASkillSubModuleTooltipWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "MAMaterialParams.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Setting/MAGameSettings.h"

void UMASkillSubModuleTooltipWidget::SetSubModule(const UMASkillModule& SubModule)
{
	const FMADisplayData DisplayData = SubModule.ResolveDisplayData(
		UMAGameSettings::Get()->GetModuleQualityData());
	SetDescription(DisplayData.DisplayName, DisplayData.Description);

	const FMAIconData& IconData = DisplayData.IconData;
	if (UMaterialInstanceDynamic* IconMaterial = IconImage->GetDynamicMaterial())
	{
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_IconTexture, IconData.Icon);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_IconColor, IconData.IconColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_InnerColor, IconData.InnerColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_FrameColor, IconData.FrameColor);
		IconMaterial->SetScalarParameterValue(PARAM_ModuleIcon_UseIcon, IconData.Icon ? 1.f : 0.f);
		IconMaterial->SetScalarParameterValue(PARAM_ModuleIcon_UseSubIcon, 0.f);
	}
	IconImage->SetVisibility(IconData.Icon
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);

	FLinearColor BackgroundColor = IconData.FrameColor;
	BackgroundColor.A = BackgroundBorder->GetBrushColor().A;
	BackgroundBorder->SetBrushColor(BackgroundColor);
}
