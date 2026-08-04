#include "GAS/Skill/StatusEffect/MASkillStatusEffect_Impulse.h"

#include "GAS/MAAbilitySystemStatics.h"

bool UMASkillStatusEffectImpulseBase::ResolvePolicy(FMASkillStatusEffectPolicy& OutPolicy) const
{
	OutPolicy.StatusEffectTag = GetStatusEffectTag();
	OutPolicy.Magnitude = Magnitude;
	OutPolicy.Duration = Duration;
	OutPolicy.SourceType = SourceType;
	AppendGrantedStateTags(FMASkillStatusEffectGrantedStateRule(true, true, true), OutPolicy.GrantedStateTags);
	return true;
}

FGameplayTag UMASkillStatusEffectKnockback::GetStatusEffectTag() const
{
	return UMAAbilitySystemStatics::GetKnockbackStatTag();
}

FGameplayTag UMASkillStatusEffectGrab::GetStatusEffectTag() const
{
	return UMAAbilitySystemStatics::GetGrabStatTag();
}

FGameplayTag UMASkillStatusEffectStagger::GetStatusEffectTag() const
{
	return UMAAbilitySystemStatics::GetStaggerStatTag();
}
