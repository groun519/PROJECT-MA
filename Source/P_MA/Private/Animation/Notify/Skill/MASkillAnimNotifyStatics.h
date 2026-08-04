#pragma once

#include "CoreMinimal.h"

class UAnimSequenceBase;
class UMASkillAbility;
class USkeletalMeshComponent;

class P_MA_API MASkillAnimNotifyStatics final
{
public:
	static UMASkillAbility* ResolveAnimationOwnerSkillAbility(
		USkeletalMeshComponent* MeshComp,
		const UAnimSequenceBase* Animation);

	static float ResolveSkillAreaScale(UMASkillAbility* SkillAbility);
	static float ResolveSkillAreaScale(
		USkeletalMeshComponent* MeshComp,
		const UAnimSequenceBase* Animation);

private:
	MASkillAnimNotifyStatics() = delete;
};
