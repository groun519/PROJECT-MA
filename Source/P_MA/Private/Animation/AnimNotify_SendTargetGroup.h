// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AnimNotify_SendTargetGroup.generated.h"

UENUM(BlueprintType)
enum class EVA_Shape : uint8
{
	Sphere,
	Box
};

/**
 * 
 */
UCLASS()
class UAnimNotify_SendTargetGroup : public UAnimNotify
{
	GENERATED_BODY()
		
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	FGameplayTag EventTag;
	
	
	/** 디버그/타겟 지점의 모양 */
	UPROPERTY(EditAnywhere, Category="Virtual Socket")
	EVA_Shape Shape = EVA_Shape::Sphere;

	/** 루트(컴포넌트) 기준 로컬 오프셋/회전 */
	UPROPERTY(EditAnywhere, Category="Virtual Socket")
	FVector  LocalOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category="Virtual Socket")
	FRotator LocalRotation = FRotator::ZeroRotator;

	/** 모양별 크기 */
	UPROPERTY(EditAnywhere, Category="Virtual Socket", meta=(ClampMin="0.0"))
	float SphereRadius = 25.f;

	UPROPERTY(EditAnywhere, Category="Virtual Socket", meta=(ClampMin="0.0"))
	FVector BoxHalfSize = FVector(20.f,12.f,12.f);

	// AnimNotify_SendTargetGroup.h
	UPROPERTY(EditAnywhere, Category="Virtual Socket|Debug", meta=(ClampMin="0.0"))
	float DebugDuration = 0.3f; // n초 동안 유지
	
	/** TargetLocation = SourceLocation + Up * Height */
	UPROPERTY(EditAnywhere, Category="Virtual Socket")
	float Height = 5.0f;

	/** 에디터 프리뷰 전용 디버그 옵션 */
	UPROPERTY(EditAnywhere, Category="Virtual Socket|Debug")
	FColor DebugColor = FColor::Cyan;

	UPROPERTY(EditAnywhere, Category="Virtual Socket|Debug", meta=(ClampMin="0.1"))
	float DebugThickness = 1.5f;

	UPROPERTY(EditAnywhere, Category="Virtual Socket|Debug")
	bool bEditorPreviewOnly = true;
};
