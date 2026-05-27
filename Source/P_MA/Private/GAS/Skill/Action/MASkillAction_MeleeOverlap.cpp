#include "GAS/Skill/Action/MASkillAction_MeleeOverlap.h"

#include "GAS/Skill/Action/MASkillAction_MeleeOverlapHelper.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Damage/MASkillDamageApplicator.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Damage/MASkillDamageResolver.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"

void UMASkillAction_MeleeOverlap::Execute(
	UMASkillAbility& OwnerAbility,
	const FGameplayEventData& EventData,
	UMASkillModuleInstance*,
	UMASkillModuleInstance* EventOwnerScope)
{
	if (!OwnerAbility.K2_HasAuthority()) return;
	if (!EventOwnerScope) return;

	const FMASkillPayloadStore& PayloadStore = EventOwnerScope->GetPayloadStore();
	const FMASkillDamageConfig DamageConfig = MASkillActionMeleeOverlap::ResolveDamageConfig(PayloadStore, DamagePayloadTag);
	const FResolvedSkillHitEffects ResolvedHitEffects = MASkillDamageResolver::Resolve(OwnerAbility, DamageConfig, PayloadStore);
	const TArray<FHitResult> HitResults = MASkillActionMeleeOverlap::ResolveHitResultsFromEventData(OwnerAbility, EventData, ResolvedHitEffects.TargetRelationMask);
	const FVector StatusEffectCenterPoint = MASkillActionMeleeOverlap::ResolveStatusEffectCenterPoint(OwnerAbility, EventData);
	MASkillDamageApplicator::ApplyHitResults(OwnerAbility, EventOwnerScope, HitResults, ResolvedHitEffects, StatusEffectCenterPoint);
}
