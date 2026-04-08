#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControlResolvedTypes.h"
#include "GAS/Skill/MASkillDamageConfig.h"

class UMASkillAbility;

class P_MA_API FMASkillCrowdControlSpecBuilder
{
public:
	static TArray<FResolvedCrowdControlEffect> BuildSpecs(UMASkillAbility& SkillAbility, const FMASkillDamageConfig& DamageConfig);
};
