// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SendNewPlayerTrans.generated.h"

UENUM(BlueprintType)
enum class EMoveType : uint8
{
	None,
	Jump,		// 도약								(ex. 잭스 q	)
	Dash,		// 방향전환 불가능한 이동기, 빠른 돌진	(ex. 아칼리 r)
	Rush,		// 방향전환 가능한 이동기, 느린 돌진		(ex. 사이온 r)
	Teleport,	// 순간이동							(ex. 카사딘 r)
};

UENUM(BlueprintType)
enum class EMovementNotifyTags : uint8{None,Start,End};

USTRUCT(BlueprintType)
struct P_MA_API FJumpData : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	// 해당 노티파이 시점의 Owner 캐릭터 위치
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector OwnerLocation = FVector();

	// 해당 노티파이 시점의 Owner 캐릭터 방향
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator OwnerRotation = FRotator();
	
	// Start -> End 타임라인 기준 소요시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartToEndTime = 0.0f;
	
	// Jump 소요시간 (Start -> End까지 몇 초 동안 이동할건가?), 단위 s
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float JumpTimeRequired = 0.0f;

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	// 네트워크 직렬화 (GameplayAbilityTargetData는 네트워크 연결이 꼭 필요해서 해줘야함.)
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << OwnerLocation;
		Ar << OwnerRotation;
		Ar << StartToEndTime;
		Ar << JumpTimeRequired;
		
		bOutSuccess = true;
		return true;
	}
};

USTRUCT(BlueprintType)
struct P_MA_API FDashData : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector OwnerLocation = FVector();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator OwnerRotation = FRotator();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DashForce = 0.f;

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	// 네트워크 직렬화 (GameplayAbilityTargetData는 네트워크 연결이 꼭 필요해서 해줘야함.)
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << OwnerLocation;
		Ar << OwnerRotation;
		
		bOutSuccess = true;
		return true;
	}
};

USTRUCT(BlueprintType)
struct P_MA_API FRushData : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector OwnerLocation = FVector();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator OwnerRotation = FRotator();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RushForce = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRotateAngle = 0.f;

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	// 네트워크 직렬화 (GameplayAbilityTargetData는 네트워크 연결이 꼭 필요해서 해줘야함.)
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << OwnerLocation;
		Ar << OwnerRotation;
		
		bOutSuccess = true;
		return true;
	}
};

/**
 * 
 */
UCLASS()
class UAnimNotify_SendNewPlayerTrans : public UAnimNotify
{
	GENERATED_BODY()
		
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMoveType MoveType = EMoveType::None;

	UPROPERTY(EditAnywhere, Category = "Movement",
		meta=(EditCondition="MoveType==EMoveType::Jump||MoveType==EMoveType::Dash", EditConditionHides))
	FName MoveSectionName = FName("MoveStart");

	UPROPERTY(EditAnywhere, Category = "Movement",
		meta=(EditCondition="MoveType!=EMoveType::None", EditConditionHides))
	EMovementNotifyTags TagType = EMovementNotifyTags::None;

	/** Type : Jump **//**
	 *	GA에서 목표 위치(커서 위치)를 받아와야 함.
	 *	받아온 목표 위치 기반으로, 이동 시간, 
	 */
	FGameplayTag GetJumpTag();
	// End 지점 섹션을 받아올 FName. (노티파이 위치는 찾기 힘듬. 섹션으로 End 처리, 이벤트 필요시 End 지점에 노티 추가)
    UPROPERTY(EditAnywhere, Category = "Movement",
        meta=(EditCondition="MoveType==EMoveType::Jump", EditConditionHides, ClampMin="0.1"))
    float JumpTimeRequired = 1.0f;

	
	/** Type : Dash **/
	FGameplayTag GetDashTag();
	UPROPERTY(EditAnywhere, Category = "Movement",
		meta=(EditCondition="MoveType==EMoveType::Dash", EditConditionHides))
	float DashForce = 100.f;

	
	/** Type : Rush **/
	FGameplayTag GetRushTag();
	UPROPERTY(EditAnywhere, Category = "Movement",
		meta=(EditCondition="MoveType==EMoveType::Rush", EditConditionHides))
	float RushForce = 100.f;
	UPROPERTY(EditAnywhere, Category = "Movement",
		meta=(EditCondition="MoveType==EMoveType::Rush", EditConditionHides))
	float MaxRotateAngle = 30.f;

	
	/** Type : Teleport **/
	FGameplayTag GetTeleportTag();
};
