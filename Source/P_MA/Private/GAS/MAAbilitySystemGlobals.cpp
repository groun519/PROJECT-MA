#include "GAS/MAAbilitySystemGlobals.h"

#include "MAGameplayAbilityTypes.h"

FGameplayEffectContext* UMAAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FMAGameplayEffectContext();
}
