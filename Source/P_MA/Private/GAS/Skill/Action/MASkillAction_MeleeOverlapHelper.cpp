#include "GAS/Skill/Action/MASkillAction_MeleeOverlapHelper.h"

#include "Engine/HitResult.h"
#include "GameplayAbilitySpec.h"
#include "GAS/Skill/MASkillDamageConfig.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"

namespace MASkillActionMeleeOverlap
{
	FMASkillDamageConfig ResolveDamageConfig(const FMASkillPayloadStore& PayloadStore, const FGameplayTag& DamagePayloadTag)
	{
		FMASkillDamageConfig DamageConfig;
		PayloadStore.TryGetStruct(DamagePayloadTag, DamageConfig);
		return DamageConfig;
	}

	TArray<FHitResult> ResolveHitResultsFromPayload(
		UMASkillAbility& OwnerAbility,
		const FGameplayEventData& Payload,
		int32 TargetRelationMask)
	{
		return OwnerAbility.GetHitResultFromVirtualSocketTargetData(Payload.TargetData, TargetRelationMask);
	}

	FVector ResolveCrowdControlCenterPoint(
		UMASkillAbility& OwnerAbility,
		const FGameplayEventData& Payload)
	{
		if (Payload.TargetData.Num() > 0 && Payload.TargetData.Data[0].IsValid())
		{
			return Payload.TargetData.Data[0]->GetOrigin().GetTranslation();
		}

		if (const AActor* AvatarActor = OwnerAbility.GetAvatarActorFromActorInfo())
		{
			return AvatarActor->GetActorLocation();
		}

		return FVector::ZeroVector;
	}

	void ApplyHitResults(
		FSkillRuntimeContext& RuntimeContext,
		const TArray<FHitResult>& HitResults,
		const FResolvedSkillHitEffects& ResolvedHitEffects,
		const FVector& CrowdControlCenterPoint)
	{
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
}
