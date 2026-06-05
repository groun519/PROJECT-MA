#include "Setting/MAGameSettings.h"

#include "GAS/Skill/MASkillGenericDataAsset.h"

const UMASkillGenericDataAsset* UMAGameSettings::GetDefaultSkillGenericDataAsset() const
{
	return DefaultSkillGenericDataAsset.LoadSynchronous();
}
