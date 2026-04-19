#include "GAS/Skill/Action/MASkillAction_MeleeOverlap.h"

#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"

void UMASkillAction_MeleeOverlap::Execute(UMASkillAbility& OwnerAbility, FSkillRuntimeContext& RuntimeContext, FMASkillPayloadStore& PayloadStore, const FGameplayEventData& Payload)
{
	if (!OwnerAbility.K2_HasAuthority()) return;

	FMASkillDamageConfig DamageConfig;
	PayloadStore.TryGetStruct(DamagePayloadTag, DamageConfig);

	const FResolvedSkillHitEffects ResolvedHitEffects = RuntimeContext.BuildResolvedHitEffects(DamageConfig);
	const TArray<FHitResult> HitResults = RuntimeContext.GetHitResultsFromPayload(Payload, ResolvedHitEffects.TargetRelationMask);
	const FVector CrowdControlCenterPoint = RuntimeContext.GetCrowdControlCenterPoint(Payload);

	TSet<AActor*> HitActors;
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor || HitActors.Contains(HitActor) || RuntimeContext.IsIgnoredActor(HitActor))
		{
			continue;
		}

		RuntimeContext.ApplyResolvedHitEffectsToHitResult(HitResult, ResolvedHitEffects, CrowdControlCenterPoint);
		HitActors.Add(HitActor);
		RuntimeContext.AddIgnoredActor(HitActor);
	}
}
