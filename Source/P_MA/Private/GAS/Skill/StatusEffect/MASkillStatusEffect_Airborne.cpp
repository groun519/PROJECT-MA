#include "GAS/Skill/StatusEffect/MASkillStatusEffect_Airborne.h"

#include "GameplayEffect.h"
#include "GAS/MAAbilitySystemStatics.h"

bool UMASkillStatusEffectAirborne::ResolvePolicy(FMASkillStatusEffectPolicy& OutPolicy) const
{
	OutPolicy.StatusEffectTag = UMAAbilitySystemStatics::GetAirborneStatTag();
	OutPolicy.Magnitude = Magnitude;
	OutPolicy.Duration = Duration;
	OutPolicy.SourceType = EMASkillStatusEffectSourceType::Instigator;
	AppendGrantedStateTags(FMASkillStatusEffectGrantedStateRule(true, true, true), OutPolicy.GrantedStateTags);
	return true;
}

void UMASkillStatusEffectAirborne::ApplyCustomPayload(FGameplayEffectSpecHandle& SpecHandle) const
{
	if (SpecHandle.IsValid() && !FMath::IsNearlyZero(RiseTime))
	{
		SpecHandle.Data->SetSetByCallerMagnitude(UMAAbilitySystemStatics::GetAirborneRiseTimeTag(), RiseTime);
	}
}
