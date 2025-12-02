// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "StructUtils/Public/InstancedStruct.h"
#include "SkillBehaviorConfig.generated.h"

class UMASkillBehavior;
class AMAProjectile_OverlapAOE;
class AMAProjectile_GroundTargetedAOE;
class AMAAbilityRangeActor;
class AMATargetActor_ChargeAtFwd;
class UMASkillVFXSet;

USTRUCT(BlueprintType)
struct FSkillBehaviorConfigBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common")
	float CooldownDuration = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common")
	float DamageMultiplier = 1.f;
};

USTRUCT(BlueprintType)
struct FConfig_Chain : public FSkillBehaviorConfigBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> DamageMultiplierMap;
};

USTRUCT(BlueprintType)
struct FConfig_Charge : public FSkillBehaviorConfigBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxChargeDuration = 3.0f;
};

USTRUCT(BlueprintType)
struct FConfig_ChargeFwd : public FConfig_Charge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMATargetActor_ChargeAtFwd> TargetActorClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinTraceDistance = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxTraceDistance = 1000.f;
};

USTRUCT(BlueprintType)
struct FConfig_ChargeTarget : public FConfig_Charge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMAAbilityRangeActor> MaxDistanceActorClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AMATargetActor_ChargeAtTarget> TargetActorClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 1000.f;;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSize = 400.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSize=10.f;
};

USTRUCT(BlueprintType)
struct FConfig_Hold : public FSkillBehaviorConfigBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHoldDuration = 2.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanMove=false;
};

USTRUCT(BlueprintType)
struct FConfig_SpawnActorAtTarget : public FSkillBehaviorConfigBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AMATargetActor_SelectLoc> TargetActorClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AMAAbilityRangeActor> RangeActorClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, TSubclassOf<AMAProjectile_GroundTargetedAOE>> ElementalProjectiles;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 1000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AbilityRange = 1000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnDelay =0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ProjectileCount = 1;
};

USTRUCT(BlueprintType)
struct FConfig_SpawnActorAtFwd : public FSkillBehaviorConfigBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, TSubclassOf<AMAProjectile_OverlapAOE>> ElementalProjectiles;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 1500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProjectileSpeed = 700.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExplodeRadius =200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnDelay =0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ProjectileCount = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MuzzleSocketName;
};

USTRUCT(BlueprintType)
struct FSkillBehaviorRegistryRow : public FTableRowBase
{
	GENERATED_BODY()

	// 1. 실행할 로직 클래스 (예: USkillBehavior_SpawnActorAtTarget)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UMASkillBehavior> BehaviorClass;

	// 2. 위 클래스가 사용할 데이터
	// 에디터에서 BehaviorClass를 선택하고, 그에 맞는 구조체(FConfig_SpawnAtTarget 등)를 선택해서 넣게 됩니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BaseStruct = "SkillBehaviorConfigBase"))
	FInstancedStruct BehaviorConfig;
};