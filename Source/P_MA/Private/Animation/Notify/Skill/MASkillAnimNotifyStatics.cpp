#include "Animation/Notify/Skill/MASkillAnimNotifyStatics.h"

#include "Animation/MAAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GAS/Skill/Area/MASkillAreaStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadAccessor.h"

UMASkillAbility* MASkillAnimNotifyStatics::ResolveAnimationOwnerSkillAbility(
	USkeletalMeshComponent* MeshComp,
	const UAnimSequenceBase* Animation)
{
	if (!MeshComp || !Animation) return nullptr;

	const UMAAnimInstance* AnimInstance = Cast<UMAAnimInstance>(MeshComp->GetAnimInstance());
	return AnimInstance ? AnimInstance->FindAnimationOwner(Animation) : nullptr;
}

float MASkillAnimNotifyStatics::ResolveSkillAreaScale(UMASkillAbility* SkillAbility)
{
	if (!SkillAbility) return 1.f;

	const FMASkillPayloadStore* SkillPayloadStore = &SkillAbility->GetAssembledModulePayloadStore();
	FMASkillPayloadAccessor Payloads(nullptr, SkillPayloadStore, nullptr);
	return MASkillAreaStatics::ResolveAreaScale(
		Payloads,
		SkillAbility->GetAbilitySystemComponentFromActorInfo());
}

float MASkillAnimNotifyStatics::ResolveSkillAreaScale(
	USkeletalMeshComponent* MeshComp,
	const UAnimSequenceBase* Animation)
{
	return ResolveSkillAreaScale(ResolveAnimationOwnerSkillAbility(MeshComp, Animation));
}
