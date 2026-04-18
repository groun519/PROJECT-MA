#include "GAS/Skill/Action/MASkillAction_MeleeOverlap.h"

#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"

void UMASkillAction_MeleeOverlap::Execute(UMASkillAbility& OwnerAbility, FSkillRuntimeContext& RuntimeContext, const FGameplayEventData& Payload)
{
	if (!OwnerAbility.K2_HasAuthority()) return;

	const TArray<FHitResult> HitResults = RuntimeContext.GetHitResultsFromPayload(Payload, &DamageConfig);
	const FResolvedSkillHitEffects ResolvedHitEffects = RuntimeContext.BuildResolvedHitEffects(&DamageConfig);
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
