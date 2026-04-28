#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "DebugShapeHelper.h"
#include "AnimNotifyState_SendTracePointPreview.generated.h"

class UDecalComponent;

UCLASS()
class P_MA_API UAnimNotifyState_SendTracePointPreview : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

private:
	UPROPERTY(EditAnywhere, Category="Gameplay Ability")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, Category="Preview")
	bool bSpawnInWorld = true;

	UPROPERTY(EditAnywhere, Category="Virtual Socket")
	EVA_Shape Shape = EVA_Shape::Circle;

	UPROPERTY(EditAnywhere, Category="Virtual Socket", meta=(EditCondition="Shape==EVA_Shape::Circle", EditConditionHides, ClampMin="0.0"))
	float Radius = 50.f;

	UPROPERTY(EditAnywhere, Category="Virtual Socket", meta=(EditCondition="Shape==EVA_Shape::Circle", EditConditionHides))
	bool bUseSector = false;

	UPROPERTY(EditAnywhere, Category="Virtual Socket", meta=(EditCondition="Shape==EVA_Shape::Circle", EditConditionHides, ClampMin="0.0", ClampMax="360.0"))
	float SectorAngle = 0.f;

	UPROPERTY(EditAnywhere, Category="Virtual Socket", meta=(EditCondition="Shape==EVA_Shape::Rect", EditConditionHides, ClampMin="0.0"))
	float Width = 50.f;

	UPROPERTY(EditAnywhere, Category="Virtual Socket", meta=(EditCondition="Shape==EVA_Shape::Rect", EditConditionHides, ClampMin="0.0"))
	float Height = 50.f;

	UPROPERTY(EditAnywhere, Category="Virtual Socket")
	FVector2D LocalOffset = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, Category="Virtual Socket")
	FRotator LocalRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category="Gameplay Ability")
	FGameplayTagContainer TriggerGameplayCueTags;

	UPROPERTY(EditAnywhere, Category="Gameplay Ability")
	bool bIgnoreOwner = true;

	UPROPERTY(EditAnywhere, Category="Gameplay Ability")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere, Category="Virtual Socket|Debug")
	FColor DebugColor = FColor::Green;

	UPROPERTY(EditAnywhere, Category="Virtual Socket|Debug", meta=(ClampMin="0.1"))
	float DebugThickness = 1.5f;

	TMap<TWeakObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UDecalComponent>> ActiveDecals;

	void DestroyPreviewDecal(USkeletalMeshComponent* MeshComp);
	void SpawnPreviewDecal(USkeletalMeshComponent* MeshComp, const UAnimSequenceBase* Animation, const FVector& WorldLocation, const FQuat& WorldRotation);
};
