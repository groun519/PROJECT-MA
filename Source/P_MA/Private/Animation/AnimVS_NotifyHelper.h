#pragma once
#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"

namespace AVS // Anim Virtual Socket
{
	enum class EShape : uint8 { Sphere, Box };

	struct FSpec
	{
		EShape   Shape = EShape::Sphere;
		FVector  LocalOffset = FVector::ZeroVector;     // 루트(컴포넌트) 기준
		FRotator LocalRotation = FRotator::ZeroRotator; // 루트 기준 회전 보정
		float    Radius = 25.f;                         // Sphere
		FVector  BoxHalfSize = FVector(20,12,12);       // Box
		FColor   Color = FColor::Cyan;
		float    Thickness = 1.5f;
	};

	inline bool IsEditorPreviewWorld(const UWorld* World)
	{
		if (!World) return false;
		const EWorldType::Type WT = World->WorldType;
		return (WT == EWorldType::Editor || WT == EWorldType::EditorPreview || WT == EWorldType::PIE);
	}

	// 루트(컴포넌트) 로컬 → 월드 변환
	inline void ResolveWorld(const USkeletalMeshComponent* MeshComp, const FSpec& S, FVector& OutLoc, FQuat& OutRot)
	{
		const FTransform CompXf = MeshComp->GetComponentTransform();
		OutLoc = CompXf.TransformPosition(S.LocalOffset);
		OutRot = CompXf.GetRotation() * S.LocalRotation.Quaternion();
	}

	// 프리뷰에서 1프레임 디버그(비지속)
	inline void DrawPreview(USkeletalMeshComponent* MeshComp, const FSpec& S, bool bEditorOnly=true)
	{
#if WITH_EDITOR
		if (!MeshComp) return;
		if (bEditorOnly && !IsEditorPreviewWorld(MeshComp->GetWorld())) return;

		FVector WLoc; FQuat WRot;
		ResolveWorld(MeshComp, S, WLoc, WRot);

		switch (S.Shape)
		{
		case EShape::Sphere:
			DrawDebugSphere(MeshComp->GetWorld(), WLoc, S.Radius, 16, S.Color, false, 0.f, 0, S.Thickness);
			break;
		case EShape::Box:
			DrawDebugBox(MeshComp->GetWorld(), WLoc, S.BoxHalfSize, WRot, S.Color, false, 0.f, 0, S.Thickness);
			break;
		}
#endif
	}
}
