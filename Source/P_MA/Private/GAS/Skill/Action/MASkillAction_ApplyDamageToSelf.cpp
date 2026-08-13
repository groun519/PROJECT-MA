#include "GAS/Skill/Action/MASkillAction_ApplyDamageToSelf.h"

#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Damage/MADamageApplicator.h"
#include "GAS/Skill/Damage/MASkillDamageResolver.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadAccess.h"

void UMASkillAction_ApplyDamageToSelf::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Ability && Scopes);
	if (!Owner.HasAuthority()) return;

	const FMASkillPayloadAccess Payloads = Event.GetPayloadAccess(*Scopes);
	if (!Payloads.Reader.IsValid()) return;

	FMASkillDamageConfig DamageConfig;
	if (!Payloads.Reader.TryGetStruct(DamagePayloadTag, DamageConfig)) return;

	const FMAResolvedDamage ResolvedDamage = MASkillDamageResolver::Resolve(*Ability, *Scopes, DamageConfig, Payloads);
	MADamageApplicator::ApplyToTargetActor(
		Owner,
		ResolvedDamage,
		Owner.GetActorLocation());
}
