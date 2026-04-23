#include "Animation/Notify/AnimNotify_SendTracePoint.h"

#include "Animation/Notify/MATracePointNotifyHelper.h"
#include "AbilitySystemBlueprintLibrary.h"

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

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner)) return;

	FGameplayEventData Data;
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
		Length,
		bIgnoreOwner,
		bDrawDebug,
		TriggerGameplayCueTags,
		ShapeWorldLocation);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Data);
}
