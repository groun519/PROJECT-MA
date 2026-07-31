#include "GAS/Skill/Action/MASkillAction_ApplyStatusEffectToSelf.h"

#include "AbilitySystemComponent.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Runtime/MASkillRuntimeRegistry.h"
#include "GAS/Skill/StatusEffect/MASkillStatusEffect.h"

void UMASkillAction_ApplyStatusEffectToSelf::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Ability && Scopes);
	if (!Owner.HasAuthority()) return;

	UAbilitySystemComponent* AbilitySystemComponent = Ability->GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent) return;

	TArray<FResolvedStatusEffect> ResolvedEffects;
	for (const TObjectPtr<UMASkillStatusEffect>& StatusEffect : StatusEffects)
	{
		if (StatusEffect)
		{
			StatusEffect->BuildResolvedEffect(*Ability, ResolvedEffects);
		}
	}

	for (const FResolvedStatusEffect& ResolvedEffect : ResolvedEffects)
	{
		if (ResolvedEffect.SpecHandle.IsValid())
		{
			const FActiveGameplayEffectHandle EffectHandle =
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*ResolvedEffect.SpecHandle.Data.Get());
			Scopes->GetRuntimeRegistry().Register(AbilitySystemComponent, EffectHandle);
		}
	}
}
