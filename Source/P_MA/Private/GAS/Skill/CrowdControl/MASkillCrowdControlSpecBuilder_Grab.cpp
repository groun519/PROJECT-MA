#include "GAS/Skill/CrowdControl/MASkillCrowdControlSpecBuilderInternal.h"

#include "GAS/MAAbilitySystemStatics.h"

bool MASkillCrowdControlSpecBuilderInternal::TryBuildGrabCrowdControlSpec(
	const FInstancedStruct& CrowdControlConfig,
	UMASkillAbility& SkillAbility,
	TArray<FResolvedCrowdControlEffect>& OutEffects)
{
	const FSkillCrowdControlGrabConfig* GrabConfig = CrowdControlConfig.GetPtr<FSkillCrowdControlGrabConfig>();
	if (!GrabConfig) return false;

	FMASkillCrowdControlEntry CrowdControlEntry;
	if (!BuildCrowdControlEntry(
		UMAAbilitySystemStatics::GetGrabStatTag(),
		GrabConfig->Magnitude,
		GrabConfig->Duration,
		GrabConfig->SourceType,
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
