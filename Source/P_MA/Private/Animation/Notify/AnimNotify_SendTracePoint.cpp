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
	FVector MeshForward = FVector::ZeroVector;
	if (!MATracePointNotify::ResolveWorldSpace(MeshComp, LocalOffset, LocalRotation, WorldLocation, MeshForward))
	{
		return;
	}

	MATracePointNotify::DrawDebugShape(World, Shape, WorldLocation, MeshForward, Radius, bUseSector, SectorAngle, Width, Height, DebugColor, DebugThickness);

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
		bIgnoreOwner,
		bDrawDebug,
		TriggerGameplayCueTags,
		WorldLocation);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Data);
}
