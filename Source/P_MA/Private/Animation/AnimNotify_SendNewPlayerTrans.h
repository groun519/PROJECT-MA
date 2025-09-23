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
enum class EMovementNotifyTags : uint8{None,Start,Damage};

USTRUCT(BlueprintType)
struct P_MA_API FJumpData : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector OwnerLocation = FVector();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator OwnerRotation = FRotator();

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

	/** Event Tag **/
	// 수정불가, ReadOnly.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	FGameplayTag EventHitTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement",
		meta=(EditCondition="MoveType==EMoveType::Dash&&MoveType==EMoveType::Rush"))
	FGameplayTag EventStartTag;

	// true		-> return .Hit Tag
	// false	-> return basic Tag
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement",
		meta=(EditCondition="MoveType==EMoveType::Jump||MoveType==EMoveType::Teleport"))
	bool bDamageTag = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement",
		meta=(EditCondition="MoveType==EMoveType::Jump||MoveType==EMoveType::Teleport"))
	EMovementNotifyTags TagType = EMovementNotifyTags::None;
	
	/** Type : Jump **//**
	 *	GA에서 목표 위치(커서 위치)를 받아와야 함.
	 *	받아온 목표 위치 기반으로, 이동 시간, 
	 */
	FGameplayTag GetJumpTag();
	
	/** Type : Dash **/
	FGameplayTag GetDashTag();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement",
		meta=(EditCondition="MoveType==EMoveType::Dash"))
	float DashForce = 100.f;

	/** Type : Rush **/
	FGameplayTag GetRushTag();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement",
		meta=(EditCondition="MoveType==EMoveType::Rush"))
	float RushForce = 100.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement",
		meta=(EditCondition="MoveType==EMoveType::Rush"))
	float MaxRotateAngle = 30.f;

	/** Type : Teleport **/
	FGameplayTag GetTeleportTag();
};
