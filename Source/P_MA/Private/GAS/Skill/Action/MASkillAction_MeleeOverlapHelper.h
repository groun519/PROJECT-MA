#pragma once

#include "CoreMinimal.h"

struct FGameplayTag;
struct FHitResult;
struct FMASkillDamageConfig;
struct FMASkillEvent;
struct FMASkillPayloadAccessor;
struct FResolvedSkillDamage;
class UMASkillAbility;

class P_MA_API MASkillActionMeleeOverlap final
{
public:
	static FMASkillDamageConfig ResolveDamageConfig(const FMASkillPayloadAccessor& Payloads, const FGameplayTag& DamagePayloadTag);
	static TArray<FHitResult> ResolveHitResultsFromEvent(
		UMASkillAbility& OwnerAbility,
		const FMASkillEvent& Event,
		int32 TargetRelationMask);
	static FVector ResolveStatusEffectCenterPoint(
		UMASkillAbility& OwnerAbility,
		const FMASkillEvent& Event);

private:
	MASkillActionMeleeOverlap() = delete;
};
