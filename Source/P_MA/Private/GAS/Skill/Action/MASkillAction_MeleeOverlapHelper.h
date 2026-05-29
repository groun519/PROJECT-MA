#pragma once

#include "CoreMinimal.h"

struct FGameplayTag;
struct FGameplayEventData;
struct FHitResult;
struct FMASkillDamageConfig;
struct FMASkillPayloadStore;
struct FResolvedSkillDamage;
class UMASkillAbility;

class P_MA_API MASkillActionMeleeOverlap final
{
public:
	static FMASkillDamageConfig ResolveDamageConfig(const FMASkillPayloadStore& PayloadStore, const FGameplayTag& DamagePayloadTag);
	static TArray<FHitResult> ResolveHitResultsFromEventData(
		UMASkillAbility& OwnerAbility,
		const FGameplayEventData& EventData,
		int32 TargetRelationMask);
	static FVector ResolveStatusEffectCenterPoint(
		UMASkillAbility& OwnerAbility,
		const FGameplayEventData& EventData);

private:
	MASkillActionMeleeOverlap() = delete;
};
