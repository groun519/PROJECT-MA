#include "Animation/Notify/Skill/AnimNotify_SendTracePoint.h"

#include "Animation/MAAnimInstance.h"
#include "Animation/Notify/Skill/MATracePointNotifyHelper.h"
#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"
#include "GAS/Skill/MASkillAbility.h"

namespace
{
	UMASkillAbility* ResolveTracePointOwnerSkillAbility(USkeletalMeshComponent* MeshComp, const UAnimSequenceBase* Animation)
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
	FQuat WorldRotation = FQuat::Identity;
	FVector MeshForward = FVector::ZeroVector;
	if (!MATracePointNotify::ResolveWorldSpace(MeshComp, LocalOffset, LocalRotation, WorldLocation, WorldRotation, MeshForward))
	{
		return;
	}

	const FVector ShapeForward = Shape == EVA_Shape::Line ? WorldRotation.GetAxisY().GetSafeNormal2D() : MeshForward;
	const FVector ShapeWorldLocation = Shape == EVA_Shape::Line ? WorldLocation + ShapeForward * (Length * 0.5f) : WorldLocation;

	MATracePointNotify::DrawDebugShape(World, Shape, ShapeWorldLocation, ShapeForward, Radius, bUseSector, SectorAngle, Width, Height, Length, DebugColor, DebugThickness);
	if (World->IsPreviewWorld() || MATracePointNotify::IsEditorPreviewWorldNoPIE(World)) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;
	UMASkillAbility* SkillAbility = ResolveTracePointOwnerSkillAbility(MeshComp, Animation);
	if (!SkillAbility) return;

	FMASkillEvent Event(EventTag);
	MATracePointNotify::AppendTargetData(
		Event,
		Shape,
		LocalOffset,
		LocalRotation,
		Radius,
		bUseSector,
		SectorAngle,
		Width,
		Height,
		Length,
		bIgnoreOwner,
		bDrawDebug,
		ShapeWorldLocation);

	UMASkillEventRoutingStatics::TryNotifySkillEvent(SkillAbility, MoveTemp(Event), SkillAbility->GetCurrentBindingScope());
}
