#include "GAS/Skill/Action/MASkillAction_ApplyDamageToPayloadTarget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Damage/MADamageApplicator.h"
#include "GAS/Skill/Damage/MASkillDamageResolver.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Payload/MASkillPayloadAccess.h"

void UMASkillAction_ApplyDamageToPayloadTarget::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Ability && Scopes);
	if (!Owner.HasAuthority()) return;

	const FMASkillPayloadAccess Payloads = Event.GetPayloadAccess(*Scopes);
	if (!Payloads.Reader.IsValid()) return;

	UObject* TargetObject = nullptr;
	if (!Payloads.Reader.TryGetObject(TargetPayloadTag, TargetObject)) return;

	AActor* TargetActor = Cast<AActor>(TargetObject);
	if (!TargetActor) return;

	FMASkillDamageConfig DamageConfig;
	if (!Payloads.Reader.TryGetStruct(DamagePayloadTag, DamageConfig)) return;

	UAbilitySystemComponent* SourceASC = Ability->GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!SourceASC || !TargetASC) return;

	float DamageMultiplier = BaseDamageMultiplier;
	for (const FMAAttributeCoefficient& Coefficient : DamageMultiplierCoefficients)
	{
		DamageMultiplier += Coefficient.ResolveValue(*SourceASC, *TargetASC, Payloads);
	}
	if (DamageMultiplier <= 0.f) return;
	DamageConfig.Scale(DamageMultiplier);

	const FMAResolvedDamage ResolvedDamage = MASkillDamageResolver::Resolve(*Ability, *Scopes, DamageConfig, Payloads);
	MADamageApplicator::ApplyToTargetActor(
		*TargetActor,
		ResolvedDamage,
		TargetActor->GetActorLocation());
}
