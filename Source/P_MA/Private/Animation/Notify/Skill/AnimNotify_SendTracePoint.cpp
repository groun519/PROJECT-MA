#include "Animation/Notify/Skill/AnimNotify_SendTracePoint.h"

#include "Animation/MAAnimInstance.h"
#include "Animation/Notify/Skill/MATracePointNotifyHelper.h"
#include "GAS/Skill/MASkillAbility.h"

namespace
{
	UMASkillAbility* ResolveAnimationOwnerSkillAbility(USkeletalMeshComponent* MeshComp, const UAnimSequenceBase* Animation)
	{
		if (!MeshComp || !Animation) return nullptr;

		const UMAAnimInstance* AnimInstance = Cast<UMAAnimInstance>(MeshComp->GetAnimInstance());
		return AnimInstance ? AnimInstance->FindAnimationOwner(Animation) : nullptr;
	}
}

void UAnimNotify_SendTracePoint::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!MeshComp) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	FVector WorldLocation = FVector::ZeroVector;
	FVector MeshForward = FVector::ZeroVector;
	if (!MATracePointNotify::ResolveWorldSpace(MeshComp, LocalOffset, LocalRotation, WorldLocation, MeshForward))
	{
		return;
	}

	MATracePointNotify::DrawDebugShape(World, Shape, WorldLocation, MeshForward, Radius, bUseSector, SectorAngle, Width, Height, DebugColor, DebugThickness);
	if (World->IsPreviewWorld() || MATracePointNotify::IsEditorPreviewWorldNoPIE(World)) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;
	UMASkillAbility* SkillAbility = ResolveAnimationOwnerSkillAbility(MeshComp, Animation);
	if (!SkillAbility) return;

	FGameplayEventData Data;
	Data.EventTag = EventTag;
	MATracePointNotify::AppendTargetData(
		Data,
		Owner,
		Shape,
		LocalOffset,
		LocalRotation,
		Radius,
		bUseSector,
		SectorAngle,
		Width,
		Height,
		bIgnoreOwner,
		bDrawDebug,
		TriggerGameplayCueTags,
		WorldLocation);

	SkillAbility->SendSkillGameplayEvent(Data, SkillAbility->GetCurrentRuntimeScope());
}


