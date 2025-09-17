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

	MeshForward = MeshComp->GetRightVector();

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
			if (!bUseSector)
			{
				DrawDebugCircle(World, WorldLoc, Radius, 32,
				false, 0.f, MeshForward,
				DebugColor, DebugThickness);
			}
			else
			{
				DrawDebugCircle(World, WorldLoc, Radius, 360,
				true, SectorAngle/2.f, MeshForward,
				DebugColor, DebugThickness);
			}
		}
		else if (DebugShape == EVA_Shape::Box)
		{
			DrawDebugRect(World, WorldLoc, Width, Height, MeshForward,
				DebugColor, DebugThickness);
		}
	}
#endif
}

void UAnimNotify_SendTracePoint::DrawDebugCircle(
	UWorld* World, const FVector& Center, float Rad,
	int32 Segments, bool bUseSect, float HalfAngleDeg,
	FVector Forward, FColor Color, float Thickness)
{
	if (!World || Segments < 3) return;

	if (!bUseSect) // 원만 사용
	{
		const float AngleStep = 2 * PI / Segments;
		FVector PrevPoint = Center + Rad * FVector(FMath::Cos(0.f), FMath::Sin(0.f), 0);

		for (int32 i = 1; i <= Segments; i++)
		{
			float Angle = i * AngleStep;
			FVector NextPoint = Center + Rad * FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0);

			DrawDebugLine(World, PrevPoint, NextPoint, Color, false, 3, 0, Thickness);
			PrevPoint = NextPoint;
		}
	}
	else // 부채꼴
	{
		Forward = Forward.GetSafeNormal2D();
		FVector Right = FVector::CrossProduct(Forward, FVector::UpVector).GetSafeNormal();

		float StartRad = FMath::DegreesToRadians(-HalfAngleDeg);
		float EndRad   = FMath::DegreesToRadians(HalfAngleDeg);
		float AngleStep = (EndRad - StartRad) / Segments;

		FVector PrevPoint = Center + Rad * (FMath::Cos(StartRad) * Forward + FMath::Sin(StartRad) * Right);

		for (int32 i = 1; i <= Segments; i++)
		{
			float Angle = StartRad + i * AngleStep;
			FVector NextPoint = Center + Rad * (FMath::Cos(Angle) * Forward + FMath::Sin(Angle) * Right);

			DrawDebugLine(World, PrevPoint, NextPoint, Color, false, 3, 0, Thickness);
			PrevPoint = NextPoint;

			if (i == 1 || i == Segments)
			{
				DrawDebugLine(World, Center, NextPoint, Color, false, 3, 0, Thickness);
			}
		}
	}
}

void UAnimNotify_SendTracePoint::DrawDebugRect(UWorld* World, const FVector& Center, float HalfX, float HalfY,
	FVector Forward, FColor Color, float Thickness)
{
	if (!World) return;

	Forward = Forward.GetSafeNormal2D();
	FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();

	FVector P1 = Center + Forward * HalfY + Right * HalfX;
	FVector P2 = Center + Forward * HalfY - Right * HalfX;
	FVector P3 = Center - Forward * HalfY - Right * HalfX;
	FVector P4 = Center - Forward * HalfY + Right * HalfX;

	DrawDebugLine(World, P1, P2, Color, false, 3, 0, Thickness);
	DrawDebugLine(World, P2, P3, Color, false, 3, 0, Thickness);
	DrawDebugLine(World, P3, P4, Color, false, 3, 0, Thickness);
	DrawDebugLine(World, P4, P1, Color, false, 3, 0, Thickness);
}
