// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"

#include "DebugShapeHelper.h"

#include "AnimNotify_SendTracePoint.generated.h"



/**
 * 
 */
UCLASS()
class UAnimNotify_SendTracePoint : public UAnimNotify
{
	GENERATED_BODY()
		
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
private:
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	FGameplayTag EventTag;
	
	// 타입
	UPROPERTY(EditAnywhere, Category="Virtual Socket")
	EVA_Shape Shape = EVA_Shape::Sphere;

	/** Sphere **/
	UPROPERTY(EditAnywhere, Category="Virtual Socket", meta=(EditCondition="Shape==EVA_Shape::Sphere", EditConditionHides, ClampMin="0.0"))
	float Radius = 50.f;
	UPROPERTY(EditAnywhere, Category="Virtual Socket", meta=(EditCondition="Shape==EVA_Shape::Sphere", EditConditionHides))
	bool bUseSector = false;
	UPROPERTY(EditAnywhere, Category="Virtual Socket", meta=(EditCondition="Shape==EVA_Shape::Sphere", EditConditionHides, ClampMin="0.0", ClampMax="360.0"))
	float SectorAngle = 0.f;
	
	/** Box **/
	UPROPERTY(EditAnywhere, Category="Virtual Socket", meta=(EditCondition="Shape==EVA_Shape::Box", EditConditionHides, ClampMin="0.0"))
	float Width = 50.f;
	UPROPERTY(EditAnywhere, Category="Virtual Socket", meta=(EditCondition="Shape==EVA_Shape::Box", EditConditionHides, ClampMin="0.0"))
	float Height = 50.f;
	
	// 어차피 Z축은 탑다운, 점프x 상황에서 필요없기에 고려x
	UPROPERTY(EditAnywhere, Category="Virtual Socket")
	FVector2D  LocalOffset	= FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, Category="Virtual Socket")
	FRotator LocalRotation	= FRotator::ZeroRotator;


	/** Debug **/
	UPROPERTY(EditAnywhere, Category="Virtual Socket|Debug")
	FColor DebugColor = FColor::Green;

	UPROPERTY(EditAnywhere, Category="Virtual Socket|Debug", meta=(ClampMin="0.1"))
	float DebugThickness = 1.5f;

	FVector MeshForward = FVector::ZeroVector;

	void DebugShapeWithEditor(UWorld* World, EVA_Shape DebugShape, FVector WorldLoc, FQuat WorldRot);
};
