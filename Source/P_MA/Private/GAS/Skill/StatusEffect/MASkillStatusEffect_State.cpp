#include "GAS/Skill/StatusEffect/MASkillStatusEffect_State.h"

#include "GAS/MAAbilitySystemStatics.h"

bool UMASkillStatusEffectStateBase::ResolvePolicy(FMASkillStatusEffectPolicy& OutPolicy) const
{
	OutPolicy.StatusEffectTag = GetStatusEffectTag();
	OutPolicy.Magnitude = 0.f;
	OutPolicy.Duration = Duration;
	OutPolicy.SourceType = EMASkillStatusEffectSourceType::Instigator;
	AppendGrantedStateTags(GetGrantedStateRule(), OutPolicy.GrantedStateTags);
	return true;
}

FGameplayTag UMASkillStatusEffectStun::GetStatusEffectTag() const
{
	return UMAAbilitySystemStatics::GetStunStatTag();
}

FMASkillStatusEffectGrantedStateRule UMASkillStatusEffectStun::GetGrantedStateRule() const
{
	return FMASkillStatusEffectGrantedStateRule(true, true, true);
}

FGameplayTag UMASkillStatusEffectRoot::GetStatusEffectTag() const
{
	return UMAAbilitySystemStatics::GetRootStatTag();
}

FMASkillStatusEffectGrantedStateRule UMASkillStatusEffectRoot::GetGrantedStateRule() const
{
	return FMASkillStatusEffectGrantedStateRule(true, false, false);
}
