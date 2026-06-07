#include "Setting/MAGameSettings.h"

#include "GAS/Skill/MASkillGenericDataAsset.h"
#include "Materials/MaterialInterface.h"

const UMASkillGenericDataAsset* UMAGameSettings::GetDefaultSkillGenericDataAsset() const
{
	return DefaultSkillGenericDataAsset.LoadSynchronous();
}

UMaterialInterface* UMAGameSettings::GetOverlayMaterial() const
{
	return OverlayMaterial.LoadSynchronous();
}
