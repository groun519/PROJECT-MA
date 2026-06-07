#include "GAS/Skill/Action/MASkillAction_ApplyGameplayEffectToSelf.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GAS/Skill/MASkillAbility.h"

void UMASkillAction_ApplyGameplayEffectToSelf::Execute(
	UMASkillAbility& OwnerAbility,
	const FGameplayEventData&,
	const FMASkillEventScopes&)
{
	if (!OwnerAbility.K2_HasAuthority()) return;

	check(GameplayEffectClass);

	UAbilitySystemComponent* AbilitySystemComponent = OwnerAbility.GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent) return;

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(&OwnerAbility);

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffectClass, EffectLevel, EffectContext);
	if (!SpecHandle.IsValid()) return;

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
