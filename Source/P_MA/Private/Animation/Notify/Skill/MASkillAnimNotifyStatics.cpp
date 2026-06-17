#include "Animation/Notify/Skill/MASkillAnimNotifyStatics.h"

#include "Animation/MAAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"

UMASkillAbility* MASkillAnimNotifyStatics::ResolveAnimationOwnerSkillAbility(
	USkeletalMeshComponent* MeshComp,
	const UAnimSequenceBase* Animation)
{
	if (!MeshComp || !Animation) return nullptr;

	const UMAAnimInstance* AnimInstance = Cast<UMAAnimInstance>(MeshComp->GetAnimInstance());
	return AnimInstance ? AnimInstance->FindAnimationOwner(Animation) : nullptr;
}
