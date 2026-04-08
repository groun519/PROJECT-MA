#include "GAS/Skill/CrowdControl/MASkillCrowdControlSpecBuilderInternal.h"

#include "GAS/MAAbilitySystemStatics.h"

bool MASkillCrowdControlSpecBuilderInternal::TryBuildStaggerCrowdControlSpec(
	const FInstancedStruct& CrowdControlConfig,
	UMASkillAbility& SkillAbility,
	TArray<FResolvedCrowdControlEffect>& OutEffects)
{
	const FSkillCrowdControlStaggerConfig* StaggerConfig = CrowdControlConfig.GetPtr<FSkillCrowdControlStaggerConfig>();
	if (!StaggerConfig) return false;

	FMASkillCrowdControlEntry CrowdControlEntry;
	if (!BuildCrowdControlEntry(
		UMAAbilitySystemStatics::GetStaggerStatTag(),
		StaggerConfig->Magnitude,
		StaggerConfig->Duration,
		StaggerConfig->SourceType,
		CrowdControlEntry))
	{
		return true;
	}

	AddResolvedCrowdControlEffect(
		MakeResolvedCrowdControlSpec(SkillAbility, CrowdControlEntry),
		CrowdControlEntry.SourceType,
		OutEffects);
	return true;
}
