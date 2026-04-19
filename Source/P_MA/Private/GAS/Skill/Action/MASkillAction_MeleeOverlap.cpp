#include "GAS/Skill/Action/MASkillAction_MeleeOverlap.h"

#include "GAS/Skill/Action/MASkillAction_MeleeOverlapHelper.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"

void UMASkillAction_MeleeOverlap::Execute(UMASkillAbility& OwnerAbility, FSkillRuntimeContext& RuntimeContext, FMASkillPayloadStore& PayloadStore, const FGameplayEventData& Payload)
{
	if (!OwnerAbility.K2_HasAuthority()) return;

	const FMASkillDamageConfig DamageConfig = MASkillActionMeleeOverlap::ResolveDamageConfig(PayloadStore, DamagePayloadTag);
	const FResolvedSkillHitEffects ResolvedHitEffects = RuntimeContext.BuildResolvedHitEffects(DamageConfig);
	const TArray<FHitResult> HitResults = MASkillActionMeleeOverlap::ResolveHitResultsFromPayload(OwnerAbility, Payload, ResolvedHitEffects.TargetRelationMask);
	const FVector CrowdControlCenterPoint = MASkillActionMeleeOverlap::ResolveCrowdControlCenterPoint(OwnerAbility, Payload);
	MASkillActionMeleeOverlap::ApplyHitResults(RuntimeContext, HitResults, ResolvedHitEffects, CrowdControlCenterPoint);
}
