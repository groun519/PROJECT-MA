#pragma once

#include "CoreMinimal.h"

struct FGameplayAbilityTargetDataHandle;
struct FMASkillWorldAreaShape;

class P_MA_API MASkillAreaStatics final
{
public:
	static const FMASkillWorldAreaShape* FindWorldShape(const FGameplayAbilityTargetDataHandle& TargetData);

	static TArray<FHitResult> ResolveHitResults(
		UWorld& World,
		AActor* SourceActor,
		const FMASkillWorldAreaShape& Area,
		int32 TargetRelationMask);

	static void DrawWorldPreview(
		UWorld& World,
		const FMASkillWorldAreaShape& Area);

private:
	MASkillAreaStatics() = delete;
};
