#include "GAS/Skill/CrowdControl/MASkillCrowdControl_Airborne.h"

#include "GameplayEffect.h"
#include "GAS/MAAbilitySystemStatics.h"

bool UMASkillCrowdControlAirborne::ResolvePolicy(FMASkillCrowdControlPolicy& OutPolicy) const
{
	OutPolicy.CrowdControlTag = UMAAbilitySystemStatics::GetAirborneStatTag();
	OutPolicy.Magnitude = Magnitude;
	OutPolicy.Duration = Duration;
	OutPolicy.SourceType = EMASkillCrowdControlSourceType::Instigator;
	AppendGrantedStateTags(FMASkillCrowdControlGrantedStateRule(true, true, true), OutPolicy.GrantedStateTags);
	return true;
}

void UMASkillCrowdControlAirborne::ApplyCustomPayload(FGameplayEffectSpecHandle& SpecHandle) const
{
	if (SpecHandle.IsValid() && !FMath::IsNearlyZero(RiseTime))
	{
		SpecHandle.Data->SetSetByCallerMagnitude(UMAAbilitySystemStatics::GetAirborneRiseTimeTag(), RiseTime);
	}
}
