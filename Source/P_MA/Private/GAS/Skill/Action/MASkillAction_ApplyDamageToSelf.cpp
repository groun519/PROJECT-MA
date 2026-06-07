#include "GAS/Skill/Action/MASkillAction_ApplyDamageToSelf.h"

#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Damage/MASkillDamageApplicator.h"
#include "GAS/Skill/Damage/MASkillDamageResolver.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"

void UMASkillAction_ApplyDamageToSelf::Execute(
	UMASkillAbility& OwnerAbility,
	const FGameplayEventData&,
	const FMASkillEventScopes& Scopes)
{
	if (!OwnerAbility.K2_HasAuthority()) return;
	if (!Scopes.EventScope) return;

	AActor* AvatarActor = OwnerAbility.GetAvatarActorFromActorInfo();
	if (!AvatarActor) return;

	const FMASkillPayloadStore& PayloadStore = Scopes.EventScope->GetPayloadStore();
	FMASkillDamageConfig DamageConfig;
	if (!PayloadStore.TryGetStruct(DamagePayloadTag, DamageConfig)) return;

	const FResolvedSkillDamage ResolvedDamage = MASkillDamageResolver::Resolve(OwnerAbility, DamageConfig, PayloadStore);
	MASkillDamageApplicator::ApplyToTargetActor(
		OwnerAbility,
		Scopes.EventScope,
		*AvatarActor,
		ResolvedDamage,
		AvatarActor->GetActorLocation());
}
