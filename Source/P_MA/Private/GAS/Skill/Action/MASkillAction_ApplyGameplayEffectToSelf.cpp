#include "GAS/Skill/Action/MASkillAction_ApplyGameplayEffectToSelf.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Runtime/MASkillRuntimeRegistry.h"
#include "GameFramework/Actor.h"

void UMASkillAction_ApplyGameplayEffectToSelf::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	if (!Owner.HasAuthority() || !GameplayEffectClass) return;

	UAbilitySystemComponent* AbilitySystemComponent = Ability
		? Ability->GetAbilitySystemComponentFromActorInfo()
		: UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(&Owner);
	if (!AbilitySystemComponent) return;

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(Ability ? static_cast<UObject*>(Ability) : &Owner);

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffectClass, EffectLevel, EffectContext);
	if (!SpecHandle.IsValid()) return;

	const FActiveGameplayEffectHandle EffectHandle =
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	if (Scopes && EffectHandle.WasSuccessfullyApplied())
	{
		Scopes->GetRuntimeRegistry().Register(AbilitySystemComponent, EffectHandle);
	}
}
