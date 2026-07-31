#include "Widget/Lobby/Loadout/LoadoutWeaponModuleButtonWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "MAMaterialParams.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Setting/MAGameSettings.h"

void ULoadoutWeaponModuleButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EquippedBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
	EquippedBorder->SetOpacity(0.f);

	ModuleButton->OnClicked.AddUniqueDynamic(this, &ULoadoutWeaponModuleButtonWidget::HandleModuleButtonClicked);
	CreateIconMaterial();
	RefreshIcon();
}

void ULoadoutWeaponModuleButtonWidget::SetModule(UMASkillModule* InModule)
{
	Module = InModule;
	RefreshIcon();
}

void ULoadoutWeaponModuleButtonWidget::SetSelected(bool bSelected)
{
	EquippedBorder->SetOpacity(bSelected ? 1.f : 0.f);
}

void ULoadoutWeaponModuleButtonWidget::HandleModuleButtonClicked()
{
	if (Module)
	{
		OnModuleSelected.Broadcast(this);
	}
}

void ULoadoutWeaponModuleButtonWidget::CreateIconMaterial()
{
	FButtonStyle ButtonStyle = ModuleButton->GetStyle();
	UMaterialInterface* BaseMaterial = Cast<UMaterialInterface>(ButtonStyle.Normal.GetResourceObject());
	if (!BaseMaterial) return;

	IconMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	ButtonStyle.Normal.SetResourceObject(IconMaterial);
	ButtonStyle.Hovered.SetResourceObject(IconMaterial);
	ButtonStyle.Pressed.SetResourceObject(IconMaterial);
	ButtonStyle.Disabled.SetResourceObject(IconMaterial);
	ModuleButton->SetStyle(ButtonStyle);
}

void ULoadoutWeaponModuleButtonWidget::RefreshIcon()
{
	if (!IconMaterial) return;

	const FMAIconData IconData = Module
		? Module->ResolveDisplayData(UMAGameSettings::Get()->GetModuleQualityData()).IconData
		: FMAIconData();

	IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_IconTexture, IconData.Icon);
	IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_SubIconTexture, IconData.SubIcon);
	IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_IconColor, IconData.IconColor);
	IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_InnerColor, IconData.InnerColor);
	IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_FrameColor, IconData.FrameColor);
	IconMaterial->SetScalarParameterValue(PARAM_ModuleIcon_UseIcon, IconData.Icon ? 1.f : 0.f);
	IconMaterial->SetScalarParameterValue(PARAM_ModuleIcon_UseSubIcon, IconData.SubIcon ? 1.f : 0.f);
}
