#include "GAS/Skill/Action/MASkillAction_ShatterFrozenTarget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/Skill/Damage/MASkillDamageApplicator.h"
#include "GAS/Skill/Damage/MASkillDamageResolver.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadAccess.h"

void UMASkillAction_ShatterFrozenTarget::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Ability && Scopes);
	if (!Owner.HasAuthority() || AdditionalDamageMultiplier <= 0.f) return;

	const FMASkillPayloadAccess Payloads = Event.GetPayloadAccess(*Scopes);
	UObject* TargetObject = nullptr;
	float AppliedDamage = 0.f;
	if (!Payloads.Reader.TryGetObject(UMAAbilitySystemStatics::GetDamageTargetTag(), TargetObject)
		|| !Payloads.Reader.TryGetScalar(UMAAbilitySystemStatics::GetAppliedDamageTag(), AppliedDamage)
		|| AppliedDamage <= 0.f)
	{
		return;
	}

	AActor* TargetActor = Cast<AActor>(TargetObject);
	if (!TargetActor) return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC || !TargetASC->HasMatchingGameplayTag(UMAAbilitySystemStatics::GetFrozenStatTag())) return;

	FMASkillDamageConfig DamageConfig;
	DamageConfig.BaseDamage = AppliedDamage * AdditionalDamageMultiplier;
	DamageConfig.DamageTypeTag = UMAAbilitySystemStatics::GetFixedDamageTypeTag();
	DamageConfig.TargetGameplayCueTags = TargetGameplayCueTags;

	// AppliedDamage already includes the triggering skill's modifiers and armor calculation.
	const FMASkillPayloadStore EmptyPayloads;
	const FResolvedSkillDamage ResolvedDamage = MASkillDamageResolver::Resolve(*Ability, DamageConfig, EmptyPayloads);
	if (!ResolvedDamage.DamageSpec.IsValid()) return;

	const FGameplayAttribute TemperatureAttribute = UMAAttributeSet::GetTemperatureAttribute();
	const float Temperature = TargetASC->GetNumericAttribute(TemperatureAttribute);
	TargetASC->ApplyModToAttribute(TemperatureAttribute, EGameplayModOp::Additive, -Temperature);

	MASkillDamageApplicator::ApplyToTargetActor(
		*Ability,
		*Scopes,
		*TargetActor,
		ResolvedDamage,
		TargetActor->GetActorLocation());
}
