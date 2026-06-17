#pragma once

#include "CoreMinimal.h"

class AActor;
class UDecalComponent;
class UMASkillAbility;
class USceneComponent;
struct FMASkillWorldAreaShape;

class P_MA_API MASkillAreaDecalStatics final
{
public:
	static UDecalComponent* Spawn(
		AActor* ComponentOwner,
		USceneComponent* AttachParent,
		const UMASkillAbility* SkillAbility,
		const FMASkillWorldAreaShape& Area);

	static void SpawnImpact(
		UMASkillAbility& SkillAbility,
		const FMASkillWorldAreaShape& Area);

	static void SetAreaTransform(
		UDecalComponent& Decal,
		const FMASkillWorldAreaShape& Area);

private:
	MASkillAreaDecalStatics() = delete;
};
