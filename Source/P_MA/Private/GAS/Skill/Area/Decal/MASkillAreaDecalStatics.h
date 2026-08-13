#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

class AActor;
class UDecalComponent;
class USceneComponent;
class UMASkillAbility;
struct FMASkillWorldAreaShape;

class P_MA_API MASkillAreaDecalStatics final
{
public:
	static UDecalComponent* SpawnPreview(
		AActor* ComponentOwner,
		USceneComponent* AttachParent,
		FGameplayTag VisualTag,
		const FMASkillWorldAreaShape& Area);

	static void SpawnImpact(
		UMASkillAbility& SkillAbility,
		const FMASkillWorldAreaShape& Area,
		FGameplayTag DamageTypeTag);
	static void SpawnImpact(
		AActor& ComponentOwner,
		const FMASkillWorldAreaShape& Area,
		FGameplayTag VisualTag);

	static void SpawnImpactLocal(
		AActor* ComponentOwner,
		FGameplayTag VisualTag,
		const FMASkillWorldAreaShape& Area);

	static void SetAreaTransform(
		UDecalComponent& Decal,
		const FMASkillWorldAreaShape& Area);

private:
	MASkillAreaDecalStatics() = delete;
	static UDecalComponent* SpawnDecal(
		AActor* ComponentOwner,
		USceneComponent* AttachParent,
		FGameplayTag VisualTag,
		const FMASkillWorldAreaShape& Area);
};
