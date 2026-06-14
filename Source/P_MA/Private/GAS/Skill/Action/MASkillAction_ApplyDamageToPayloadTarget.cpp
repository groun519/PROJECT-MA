#include "GAS/Skill/Action/MASkillAction_ApplyDamageToPayloadTarget.h"

#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Damage/MASkillDamageApplicator.h"
#include "GAS/Skill/Damage/MASkillDamageResolver.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadAccessor.h"

void UMASkillAction_ApplyDamageToPayloadTarget::Execute(
	UMASkillAbility& OwnerAbility,
	const FMASkillEvent& Event,
	const FMASkillScopes& Scopes)
{
		if (!OwnerAbility.K2_HasAuthority()) return;

	const FMASkillPayloadAccessor Payloads = Event.GetPayloadAccess(Scopes);
	if (!Payloads.IsValid()) return;

	UObject* TargetObject = nullptr;
	if (!Payloads.TryGetObject(TargetPayloadTag, TargetObject)) return;

	AActor* TargetActor = Cast<AActor>(TargetObject);
	if (!TargetActor) return;

	FMASkillDamageConfig DamageConfig;
	if (!Payloads.TryGetStruct(DamagePayloadTag, DamageConfig)) return;

	const FResolvedSkillDamage ResolvedDamage = MASkillDamageResolver::Resolve(OwnerAbility, DamageConfig, Payloads);
	MASkillDamageApplicator::ApplyToTargetActor(
		OwnerAbility,
		Scopes,
		*TargetActor,
		ResolvedDamage,
		TargetActor->GetActorLocation());
}
