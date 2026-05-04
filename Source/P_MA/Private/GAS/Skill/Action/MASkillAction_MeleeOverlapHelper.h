#pragma once

#include "CoreMinimal.h"

struct FGameplayTag;
struct FGameplayEventData;
struct FHitResult;
struct FMASkillDamageConfig;
struct FMASkillPayloadStore;
struct FResolvedSkillHitEffects;
class UMASkillAbility;

class P_MA_API MASkillActionMeleeOverlap final
{
public:
	static FMASkillDamageConfig ResolveDamageConfig(const FMASkillPayloadStore& PayloadStore, const FGameplayTag& DamagePayloadTag);
	static TArray<FHitResult> ResolveHitResultsFromPayload(
		UMASkillAbility& OwnerAbility,
		const FGameplayEventData& Payload,
		int32 TargetRelationMask);
	static FVector ResolveStatusEffectCenterPoint(
		UMASkillAbility& OwnerAbility,
		const FGameplayEventData& Payload);

private:
	MASkillActionMeleeOverlap() = delete;
};
