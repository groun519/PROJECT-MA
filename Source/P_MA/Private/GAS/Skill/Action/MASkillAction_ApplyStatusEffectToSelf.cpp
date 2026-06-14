#include "GAS/Skill/Action/MASkillAction_ApplyStatusEffectToSelf.h"

#include "AbilitySystemComponent.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Runtime/MASkillRuntimeRegistry.h"
#include "GAS/Skill/StatusEffect/MASkillStatusEffect.h"

void UMASkillAction_ApplyStatusEffectToSelf::Execute(
	UMASkillAbility& OwnerAbility,
	const FMASkillEvent&,
	const FMASkillScopes& Scopes)
{
	if (!OwnerAbility.K2_HasAuthority()) return;

	UAbilitySystemComponent* AbilitySystemComponent = OwnerAbility.GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent) return;

	TArray<FResolvedStatusEffect> ResolvedEffects;
	for (const TObjectPtr<UMASkillStatusEffect>& StatusEffect : StatusEffects)
	{
		if (StatusEffect)
		{
			StatusEffect->BuildResolvedEffect(OwnerAbility, ResolvedEffects);
		}
	}

	for (const FResolvedStatusEffect& ResolvedEffect : ResolvedEffects)
	{
		if (ResolvedEffect.SpecHandle.IsValid())
		{
			const FActiveGameplayEffectHandle EffectHandle =
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*ResolvedEffect.SpecHandle.Data.Get());
			Scopes.GetRuntimeRegistry().Register(AbilitySystemComponent, EffectHandle);
		}
	}
}
