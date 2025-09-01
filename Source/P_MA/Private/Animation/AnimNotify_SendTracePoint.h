// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "AnimNotify_SendTracePoint.generated.h"

UENUM(BlueprintType)
enum class EVA_Shape : uint8
{
	Sphere,
	Box
};

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

	/** 에디터 프리뷰 전용 디버그 옵션 */
	UPROPERTY(EditAnywhere, Category="Virtual Socket|Debug")
	FColor DebugColor = FColor::Cyan;

	UPROPERTY(EditAnywhere, Category="Virtual Socket|Debug", meta=(ClampMin="0.1"))
	float DebugThickness = 1.5f;
};
