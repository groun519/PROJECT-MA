#include "Animation/Notify/Skill/MATracePointNotifyHelper.h"

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "VirtualSocketTargetData.h"

namespace MATracePointNotify
{
	bool ResolveWorldSpace(
		USkeletalMeshComponent* MeshComp,
		const FVector2D& LocalOffset,
		const FRotator& LocalRotation,
		FVector& OutWorldLocation,
		FVector& OutMeshForward)
	{
		FQuat UnusedWorldRotation = FQuat::Identity;
		return ResolveWorldSpace(MeshComp, LocalOffset, LocalRotation, OutWorldLocation, UnusedWorldRotation, OutMeshForward);
	}

	bool ResolveWorldSpace(
		USkeletalMeshComponent* MeshComp,
		const FVector2D& LocalOffset,
		const FRotator& LocalRotation,
		FVector& OutWorldLocation,
		FQuat& OutWorldRotation,
		FVector& OutMeshForward)
	{
		if (!MeshComp) return false;

		OutMeshForward = MeshComp->GetRightVector();

		FTransform BaseWorldTransform;
		const int32 NumBones = MeshComp->GetNumBones();
		if (NumBones > 0)
		{
			const FName RootBoneName = MeshComp->GetBoneName(0);
			BaseWorldTransform = FTransform(
				MeshComp->GetBoneQuaternion(RootBoneName),
				MeshComp->GetBoneLocation(RootBoneName),
				FVector::OneVector);
		}
		else
		{
			BaseWorldTransform = MeshComp->GetComponentTransform();
		}

		OutWorldLocation = BaseWorldTransform.TransformPosition(FVector(LocalOffset.X, LocalOffset.Y, 0.f));
		OutWorldRotation = BaseWorldTransform.GetRotation() * LocalRotation.Quaternion();
		return true;
	}

	void AppendTargetData(
		FGameplayEventData& OutData,
		AActor* Owner,
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
		const FVector& WorldLocation)
	{
		auto* LocationInfo = new FGameplayAbilityTargetData_LocationInfo();
		LocationInfo->SourceLocation.LiteralTransform.SetLocation(WorldLocation);
		OutData.TargetData.Add(LocationInfo);
		OutData.Instigator = Owner;

		if (Shape == EVA_Shape::Line || Shape == EVA_Shape::None) return;

		auto* VirtualSocketData = new FGameplayAbilityTargetData_VirtualSocket();
		VirtualSocketData->Shape = Shape;
		VirtualSocketData->LocalOffset = FVector(LocalOffset.X, LocalOffset.Y, 0.f);
		VirtualSocketData->LocalRotation = LocalRotation;
		VirtualSocketData->SphereRadius = Radius;
		VirtualSocketData->BoxHalfSize =
			Shape == EVA_Shape::Rect ? FVector(Height, Width, 100.f)
			: FVector::ZeroVector;
		VirtualSocketData->bUseSector = bUseSector;
		VirtualSocketData->SectorAngle = SectorAngle;
		VirtualSocketData->bIgnoreOwner = bIgnoreOwner;
		VirtualSocketData->bDrawDebug = bDrawDebug;
		OutData.TargetData.Add(VirtualSocketData);
	}

	bool IsEditorPreviewWorldNoPIE(const UWorld* World)
	{
#if WITH_EDITOR
		if (!World) return false;
		return World->WorldType == EWorldType::Editor || World->WorldType == EWorldType::EditorPreview;
#else
		return false;
#endif
	}

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
		float DebugThickness)
	{
#if WITH_EDITOR
		// Intentional: editor preview visualization is always shown for authoring,
		// and is not controlled by runtime bDrawDebug payload settings.
		if (!IsEditorPreviewWorldNoPIE(World)) return;

		FlushPersistentDebugLines(World);
		if (Shape == EVA_Shape::Circle)
		{
			FDebugShapeHelper::DrawDebugSectorableCircle(
				World,
				WorldLocation,
				Radius,
				bUseSector ? 360 : 32,
				bUseSector,
				bUseSector ? SectorAngle * 0.5f : 0.f,
				MeshForward,
				DebugColor,
				DebugThickness);
			return;
		}

		if (Shape == EVA_Shape::Rect)
		{
			FDebugShapeHelper::DrawDebugRect(
				World,
				WorldLocation,
				Height,
				Width,
				MeshForward,
				DebugColor,
				DebugThickness);
			return;
		}

		if (Shape == EVA_Shape::Line)
		{
			FDebugShapeHelper::DrawDebugRect(
				World,
				WorldLocation,
				Length * 0.5f,
				Radius,
				MeshForward,
				DebugColor,
				DebugThickness);
		}
#endif
	}
}

