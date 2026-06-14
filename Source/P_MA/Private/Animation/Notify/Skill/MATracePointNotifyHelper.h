#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "DebugShapeHelper.h"

class USkeletalMeshComponent;
class UWorld;
struct FMASkillEvent;

namespace MATracePointNotify
{
	bool ResolveWorldSpace(
		USkeletalMeshComponent* MeshComp,
		const FVector2D& LocalOffset,
		const FRotator& LocalRotation,
		FVector& OutWorldLocation,
		FVector& OutMeshForward);

	bool ResolveWorldSpace(
		USkeletalMeshComponent* MeshComp,
		const FVector2D& LocalOffset,
		const FRotator& LocalRotation,
		FVector& OutWorldLocation,
		FQuat& OutWorldRotation,
		FVector& OutMeshForward);

	void AppendTargetData(
		FMASkillEvent& OutEvent,
		EVA_Shape Shape,
		const FVector2D& LocalOffset,
		const FRotator& LocalRotation,
		float Radius,
		bool bUseSector,
		float SectorAngle,
		float Width,
		float Height,
		float Length,
		bool bIgnoreOwner,
		bool bDrawDebug,
		const FVector& WorldLocation);

	void DrawDebugShape(
		UWorld* World,
		EVA_Shape Shape,
		const FVector& WorldLocation,
		const FVector& MeshForward,
		float Radius,
		bool bUseSector,
		float SectorAngle,
		float Width,
		float Height,
		float Length,
		const FColor& DebugColor,
		float DebugThickness);

	bool IsEditorPreviewWorldNoPIE(const UWorld* World);
}
