#include "GAS/Skill/CrowdControl/MASkillCrowdControlSpecBuilderInternal.h"

#include "GAS/MAAbilitySystemStatics.h"

bool MASkillCrowdControlSpecBuilderInternal::TryBuildStunCrowdControlSpec(
	const FInstancedStruct& CrowdControlConfig,
	UMASkillAbility& SkillAbility,
	TArray<FResolvedCrowdControlEffect>& OutEffects)
{
	const FSkillCrowdControlStunConfig* StunConfig = CrowdControlConfig.GetPtr<FSkillCrowdControlStunConfig>();
	if (!StunConfig) return false;

	FMASkillCrowdControlEntry CrowdControlEntry;
	if (!BuildCrowdControlEntry(
		UMAAbilitySystemStatics::GetStunStatTag(),
		0.f,
		StunConfig->Duration,
		EMASkillCrowdControlSourceType::Instigator,
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
