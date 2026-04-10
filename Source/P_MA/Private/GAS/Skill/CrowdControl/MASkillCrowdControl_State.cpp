#include "GAS/Skill/CrowdControl/MASkillCrowdControl_State.h"

#include "GAS/MAAbilitySystemStatics.h"

bool UMASkillCrowdControlStateBase::ResolvePolicy(FMASkillCrowdControlPolicy& OutPolicy) const
{
	OutPolicy.CrowdControlTag = GetCrowdControlTag();
	OutPolicy.Magnitude = 0.f;
	OutPolicy.Duration = Duration;
	OutPolicy.SourceType = EMASkillCrowdControlSourceType::Instigator;
	AppendGrantedStateTags(GetGrantedStateRule(), OutPolicy.GrantedStateTags);
	return true;
}

FGameplayTag UMASkillCrowdControlStun::GetCrowdControlTag() const
{
	return UMAAbilitySystemStatics::GetStunStatTag();
}

FMASkillCrowdControlGrantedStateRule UMASkillCrowdControlStun::GetGrantedStateRule() const
{
	return FMASkillCrowdControlGrantedStateRule(true, true, true);
}

FGameplayTag UMASkillCrowdControlRoot::GetCrowdControlTag() const
{
	return UMAAbilitySystemStatics::GetRootStatTag();
}

FMASkillCrowdControlGrantedStateRule UMASkillCrowdControlRoot::GetGrantedStateRule() const
{
	return FMASkillCrowdControlGrantedStateRule(true, false, false);
}
