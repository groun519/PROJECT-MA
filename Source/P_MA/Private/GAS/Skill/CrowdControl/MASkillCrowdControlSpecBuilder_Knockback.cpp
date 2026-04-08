#include "GAS/Skill/CrowdControl/MASkillCrowdControlSpecBuilderInternal.h"

#include "GAS/MAAbilitySystemStatics.h"

bool MASkillCrowdControlSpecBuilderInternal::TryBuildKnockbackCrowdControlSpec(
	const FInstancedStruct& CrowdControlConfig,
	UMASkillAbility& SkillAbility,
	TArray<FResolvedCrowdControlEffect>& OutEffects)
{
	const FSkillCrowdControlKnockbackConfig* KnockbackConfig = CrowdControlConfig.GetPtr<FSkillCrowdControlKnockbackConfig>();
	if (!KnockbackConfig) return false;

	FMASkillCrowdControlEntry CrowdControlEntry;
	if (!BuildCrowdControlEntry(
		UMAAbilitySystemStatics::GetKnockbackStatTag(),
		KnockbackConfig->Magnitude,
		KnockbackConfig->Duration,
		KnockbackConfig->SourceType,
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
