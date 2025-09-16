// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_SendTracePoint.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"

void UAnimNotify_SendTracePoint::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!MeshComp) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	FTransform BaseWorldXf; // 루트 본의 월드 트랜스폼
	{
		// 안전 폴백: 본이 없거나 얻기 실패 시 컴포넌트 트랜스폼 사용
		const int32 NumBones = MeshComp->GetNumBones();
		if (NumBones > 0)
		{
			const FName RootBoneName = MeshComp->GetBoneName(0); // 스켈레톤 루트 본
			const FVector RootPos    = MeshComp->GetBoneLocation(RootBoneName);     // 월드
			const FQuat   RootQuat   = MeshComp->GetBoneQuaternion(RootBoneName);   // 월드
			BaseWorldXf = FTransform(RootQuat, RootPos, FVector::OneVector);
		}
		else
		{
			BaseWorldXf = MeshComp->GetComponentTransform(); // 폴백
		}
	}
	
	// 로컬(노티 값) → 월드
	const FVector WLoc = BaseWorldXf.TransformPosition(FVector(LocalOffset.X, LocalOffset.Y, 0));
	const FQuat   WRot = BaseWorldXf.GetRotation() * LocalRotation.Quaternion();
	
	DebugShapeWithEditor(World, Shape, WLoc, WRot);

	// --- ASC가 있을 때만 이벤트 송신 (에디터 프리뷰에서 return로 막지 않음) ---
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner))
		{
			FGameplayEventData Data;
			{
				auto* LocationInfo = new FGameplayAbilityTargetData_LocationInfo();
				LocationInfo->SourceLocation.LiteralTransform.SetLocation(WLoc);
				LocationInfo->TargetLocation.LiteralTransform.SetLocation(WLoc + FVector::UpVector * 150);
				Data.TargetData.Add(LocationInfo);
			}

			{
				auto* VSData = new FGameplayAbilityTargetData_VirtualSocket();
				VSData->Shape        = Shape;
				VSData->LocalOffset  = FVector(LocalOffset.X, LocalOffset.Y, 0);
				VSData->LocalRotation= LocalRotation;
				VSData->SphereRadius = Radius;
				VSData->BoxHalfSize  = FVector(Width, Height, 100);

				Data.TargetData.Add(VSData);
			}

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Data);
		}
	}
}





//** Debug Section **//

namespace
{
	inline bool IsEditorPreviewWorld_NoPIE(const UWorld* World)
	{
#if WITH_EDITOR
		if (!World) return false;
		const EWorldType::Type WT = World->WorldType;
		return (WT == EWorldType::Editor || WT == EWorldType::EditorPreview);
#else
		return false;
#endif
	}
}

void UAnimNotify_SendTracePoint::DebugShapeWithEditor(UWorld* World, EVA_Shape DebugShape, FVector WorldLoc, FQuat WorldRot)
{
	// 에디터 프리뷰: Notify 통과 프레임에만 1프레임 디버그
#if WITH_EDITOR
	if (IsEditorPreviewWorld_NoPIE(World))
	{
		FlushPersistentDebugLines(World);

		if (DebugShape == EVA_Shape::Sphere)
		{
			DrawDebugSphere(World, WorldLoc, Radius, 16,
				DebugColor, false, 3, 0, DebugThickness);

			/*if (bUseSector)
			{
				FRotator SectorRotL = FRotator(WorldRot.X, WorldRot.Y, WorldRot.Z - SectorAngle/2);
				FVector EndLocL = WorldLoc + SectorRotL.Vector() * Radius;
				DrawDebugLine(World, WorldLoc, EndLocL,
					DebugColor, false, 3, 0, DebugThickness);
				
				FRotator SectorRotR = FRotator(WorldRot.X, WorldRot.Y, WorldRot.Z + SectorAngle/2);
				FVector EndLocR = WorldLoc + SectorRotR.Vector() * Radius;
				DrawDebugLine(World, WorldLoc, EndLocR,
					DebugColor, false, 3, 0, DebugThickness);
			}*/
		}
		else if (DebugShape == EVA_Shape::Box)
		{
			DrawDebugBox(World, WorldLoc, FVector(Width, Height, 100), WorldRot,
				DebugColor, false, 3, 0, DebugThickness);
		}
	}
#endif
}