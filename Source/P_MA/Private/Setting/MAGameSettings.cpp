#include "Setting/MAGameSettings.h"

#include "GAS/Elemental/MAElementalConfigData.h"
#include "GAS/Skill/MASkillGenericDataAsset.h"
#include "GAS/Skill/Module/MAModuleQualityData.h"
#include "Materials/MaterialInterface.h"

const UMASkillGenericDataAsset* UMAGameSettings::GetDefaultSkillGenericDataAsset() const
{
	return DefaultSkillGenericDataAsset.LoadSynchronous();
}

const UMAModuleQualityData* UMAGameSettings::GetModuleQualityData() const
{
	return ModuleQualityData.LoadSynchronous();
}

const UMAElementalConfigData* UMAGameSettings::GetElementalConfigData() const
{
	return ElementalConfigData.LoadSynchronous();
}

UMaterialInterface* UMAGameSettings::GetOverlayMaterial() const
{
	return OverlayMaterial.LoadSynchronous();
}
