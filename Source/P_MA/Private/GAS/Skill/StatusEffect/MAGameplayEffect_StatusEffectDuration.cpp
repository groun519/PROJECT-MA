#include "GAS/Skill/StatusEffect/MAGameplayEffect_StatusEffectDuration.h"

UMAGameplayEffect_StatusEffectDuration::UMAGameplayEffect_StatusEffectDuration()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
}

UMAGameplayEffect_StatusEffectInfinite::UMAGameplayEffect_StatusEffectInfinite()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
}
