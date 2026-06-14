#include "GAS/Skill/Action/MASkillAction_ApplyDamageToSelf.h"

#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Damage/MASkillDamageApplicator.h"
#include "GAS/Skill/Damage/MASkillDamageResolver.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadAccessor.h"

void UMASkillAction_ApplyDamageToSelf::Execute(
	UMASkillAbility& OwnerAbility,
	const FMASkillEvent& Event,
	const FMASkillScopes& Scopes)
{
	if (!OwnerAbility.K2_HasAuthority()) return;

	AActor* AvatarActor = OwnerAbility.GetAvatarActorFromActorInfo();
	if (!AvatarActor) return;

	const FMASkillPayloadAccessor Payloads = Event.GetPayloadAccess(Scopes);
	if (!Payloads.IsValid()) return;

	FMASkillDamageConfig DamageConfig;
	if (!Payloads.TryGetStruct(DamagePayloadTag, DamageConfig)) return;

	const FResolvedSkillDamage ResolvedDamage = MASkillDamageResolver::Resolve(OwnerAbility, DamageConfig, Payloads);
	MASkillDamageApplicator::ApplyToTargetActor(
		OwnerAbility,
		Scopes,
		*AvatarActor,
		ResolvedDamage,
		AvatarActor->GetActorLocation());
}
