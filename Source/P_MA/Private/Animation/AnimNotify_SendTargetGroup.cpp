// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_SendTargetGroup.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"

namespace
{
	inline bool IsEditorPreviewWorld(const UWorld* World)
	{
#if WITH_EDITOR
		if (!World) return false;
		const EWorldType::Type WT = World->WorldType;
		return (WT == EWorldType::Editor || WT == EWorldType::EditorPreview || WT == EWorldType::PIE);
#else
		return false;
#endif
	}
}

void UAnimNotify_SendTargetGroup::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner && !UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner)) return;

	// 루트(컴포넌트) 기준 로컬 → 월드
	const FTransform CompXf = MeshComp->GetComponentTransform();
	const FVector   WLoc = CompXf.TransformPosition(LocalOffset);
	const FQuat     WRot = CompXf.GetRotation() * LocalRotation.Quaternion();

	// Target = 위로 Height 만큼 (기존 의미 유지)
	const FVector UpDir = FVector::UpVector; // 필요시 전방 정렬 등으로 확장 가능
	const FVector WTarget = WLoc + UpDir * Height;

	// Ability로 보낼 TargetData(한 지점)
	FGameplayEventData Data;
	{
		// GAS가 소유권 가짐(원본과 동일 패턴)
		FGameplayAbilityTargetData_LocationInfo* LocationInfo = new FGameplayAbilityTargetData_LocationInfo();
		LocationInfo->SourceLocation.LiteralTransform.SetLocation(WLoc);
		LocationInfo->TargetLocation.LiteralTransform.SetLocation(WTarget);
		Data.TargetData.Add(LocationInfo);
	}

	// 에디터 프리뷰: Notify 통과 프레임에만 1프레임 디버그
#if WITH_EDITOR
	// 오래 남은 선 지움
	FlushPersistentDebugLines(MeshComp->GetWorld());
	
	const bool bPersistent = DebugDuration > 0.f;
	const float LifeTime   = DebugDuration; // n초
	
	if (!bEditorPreviewOnly || IsEditorPreviewWorld(MeshComp->GetWorld()))
	{
		switch (Shape)
		{
		case EVA_Shape::Sphere:
			DrawDebugSphere(MeshComp->GetWorld(), WLoc, SphereRadius, 16,
				DebugColor, bPersistent, LifeTime, 0, DebugThickness);
			break;
		case EVA_Shape::Box:
			DrawDebugBox(MeshComp->GetWorld(), WLoc, BoxHalfSize, WRot,
				DebugColor, bPersistent, LifeTime, 0, DebugThickness);
			break;
		}
		// 상단 방향선(선택)
		DrawDebugLine(MeshComp->GetWorld(), WLoc, WTarget,
			DebugColor, bPersistent, LifeTime, 0, DebugThickness);
	}
#endif

	// 이벤트 송신(원본 유지)
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Data);
}
