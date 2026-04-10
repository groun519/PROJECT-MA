#include "GAS/Skill/CrowdControl/MASkillCrowdControl_Impulse.h"

#include "GAS/MAAbilitySystemStatics.h"

bool UMASkillCrowdControlImpulseBase::ResolvePolicy(FMASkillCrowdControlPolicy& OutPolicy) const
{
	OutPolicy.CrowdControlTag = GetCrowdControlTag();
	OutPolicy.Magnitude = Magnitude;
	OutPolicy.Duration = Duration;
	OutPolicy.SourceType = SourceType;
	AppendGrantedStateTags(FMASkillCrowdControlGrantedStateRule(true, true, true), OutPolicy.GrantedStateTags);
	return true;
}

FGameplayTag UMASkillCrowdControlKnockback::GetCrowdControlTag() const
{
	return UMAAbilitySystemStatics::GetKnockbackStatTag();
}

FGameplayTag UMASkillCrowdControlGrab::GetCrowdControlTag() const
{
	return UMAAbilitySystemStatics::GetGrabStatTag();
}

FGameplayTag UMASkillCrowdControlStagger::GetCrowdControlTag() const
{
	return UMAAbilitySystemStatics::GetStaggerStatTag();
}
