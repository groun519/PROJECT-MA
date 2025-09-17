// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "EVA_Shape.h"
#include "AnimNotify_SendTracePoint.generated.h"

USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_VirtualSocket : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EVA_Shape Shape = EVA_Shape::Sphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LocalOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator LocalRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SphereRadius = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BoxHalfSize = FVector(20.f,12.f,12.f);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << Shape;
		Ar << LocalOffset;
		Ar << LocalRotation;
		Ar << SphereRadius;
		Ar << BoxHalfSize;
		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_VirtualSocket>
	: public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_VirtualSocket>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

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
	void DrawDebugCircle(
		UWorld* World,
		const FVector& Center,
		float Rad,
		int32 Segments,
		bool bUseSect = false,
		float HalfAngleDeg = 0.f,
		FVector Forward = FVector::ForwardVector,
		FColor Color = FColor::Green,
		float Thickness = 1.f
		);
	void DrawDebugRect(
		UWorld* World,
		const FVector& Center,
		float HalfX,
		float HalfY,
		FVector Forward,
		FColor Color = FColor::Blue,
		float Thickness = 1.f
		);
};
