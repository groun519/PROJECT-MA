#pragma once

#include "CoreMinimal.h"

struct FGameplayTag;
struct FGameplayEventData;
struct FHitResult;
struct FMASkillDamageConfig;
struct FMASkillPayloadStore;
struct FResolvedSkillHitEffects;
class UMASkillAbility;

namespace MASkillActionMeleeOverlap
{
	P_MA_API FMASkillDamageConfig ResolveDamageConfig(const FMASkillPayloadStore& PayloadStore, const FGameplayTag& DamagePayloadTag);
	P_MA_API TArray<FHitResult> ResolveHitResultsFromPayload(
		UMASkillAbility& OwnerAbility,
		const FGameplayEventData& Payload,
		int32 TargetRelationMask);
	P_MA_API FVector ResolveStatusEffectCenterPoint(
		UMASkillAbility& OwnerAbility,
		const FGameplayEventData& Payload);
	P_MA_API void ApplyHitResults(
		UMASkillAbility& OwnerAbility,
		const TArray<FHitResult>& HitResults,
		const FResolvedSkillHitEffects& ResolvedHitEffects,
		const FVector& StatusEffectCenterPoint);
}
