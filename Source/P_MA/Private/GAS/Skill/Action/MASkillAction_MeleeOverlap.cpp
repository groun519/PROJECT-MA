#include "GAS/Skill/Action/MASkillAction_MeleeOverlap.h"

#include "GAS/Skill/Action/MASkillAction_MeleeOverlapHelper.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Damage/MASkillDamageApplicator.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Damage/MASkillDamageResolver.h"

void UMASkillAction_MeleeOverlap::Execute(UMASkillAbility& OwnerAbility, const FGameplayEventData& Payload)
{
	if (!OwnerAbility.K2_HasAuthority()) return;

	const FMASkillPayloadStore& PayloadStore = OwnerAbility.GetPayloadStore();
	const FMASkillDamageConfig DamageConfig = MASkillActionMeleeOverlap::ResolveDamageConfig(PayloadStore, DamagePayloadTag);
	const FResolvedSkillHitEffects ResolvedHitEffects = MASkillDamageResolver::Resolve(OwnerAbility, DamageConfig);
	const TArray<FHitResult> HitResults = MASkillActionMeleeOverlap::ResolveHitResultsFromPayload(OwnerAbility, Payload, ResolvedHitEffects.TargetRelationMask);
	const FVector StatusEffectCenterPoint = MASkillActionMeleeOverlap::ResolveStatusEffectCenterPoint(OwnerAbility, Payload);
	MASkillDamageApplicator::ApplyHitResults(OwnerAbility, HitResults, ResolvedHitEffects, StatusEffectCenterPoint);
}
