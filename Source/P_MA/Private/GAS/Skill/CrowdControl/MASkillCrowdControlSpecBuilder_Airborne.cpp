#include "GAS/Skill/CrowdControl/MASkillCrowdControlSpecBuilderInternal.h"

#include "GAS/MAAbilitySystemStatics.h"

bool MASkillCrowdControlSpecBuilderInternal::TryBuildAirborneCrowdControlSpec(
	const FInstancedStruct& CrowdControlConfig,
	UMASkillAbility& SkillAbility,
	TArray<FResolvedCrowdControlEffect>& OutEffects)
{
	const FSkillCrowdControlAirborneConfig* AirborneConfig = CrowdControlConfig.GetPtr<FSkillCrowdControlAirborneConfig>();
	if (!AirborneConfig) return false;

	FMASkillCrowdControlEntry CrowdControlEntry;
	if (!BuildCrowdControlEntry(
		UMAAbilitySystemStatics::GetAirborneStatTag(),
		AirborneConfig->Magnitude,
		AirborneConfig->Duration,
		EMASkillCrowdControlSourceType::Instigator,
		CrowdControlEntry))
	{
		return true;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeResolvedCrowdControlSpec(SkillAbility, CrowdControlEntry);
	if (SpecHandle.IsValid() && !FMath::IsNearlyZero(AirborneConfig->RiseTime))
	{
		SpecHandle.Data->SetSetByCallerMagnitude(UMAAbilitySystemStatics::GetAirborneRiseTimeTag(), AirborneConfig->RiseTime);
	}

	AddResolvedCrowdControlEffect(SpecHandle, CrowdControlEntry.SourceType, OutEffects);
	return true;
}
