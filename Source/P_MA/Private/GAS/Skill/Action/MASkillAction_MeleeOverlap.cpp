#include "GAS/Skill/Action/MASkillAction_MeleeOverlap.h"

#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"

void UMASkillAction_MeleeOverlap::Execute(FSkillRuntimeContext& RuntimeContext, const FGameplayEventData& Payload)
{
	if (!RuntimeContext.HasAuthority()) return;

	const TArray<FHitResult> HitResults = RuntimeContext.GetHitResultsFromPayload(Payload);

	TSet<AActor*> HitActors;
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor || HitActors.Contains(HitActor) || RuntimeContext.IsIgnoredActor(HitActor))
		{
			continue;
		}

		RuntimeContext.ApplyDamageToHitResult(HitResult, &DamageConfig);
		HitActors.Add(HitActor);
		RuntimeContext.AddIgnoredActor(HitActor);
	}
}
