#include "GAS/Skill/Action/MASkillAction_ApplyStatusEffectToSelf.h"

#include "AbilitySystemComponent.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/StatusEffect/MASkillStatusEffect.h"

void UMASkillAction_ApplyStatusEffectToSelf::Execute(
	UMASkillAbility& OwnerAbility,
	const FGameplayEventData&,
	const FMASkillEventScopes&)
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
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*ResolvedEffect.SpecHandle.Data.Get());
		}
	}
}
