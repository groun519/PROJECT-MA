// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MAGameplayAbility_SkillBase.h"
#include "Engine/DataTable.h"
#include "StructUtils/Public/InstancedStruct.h"
#include "SkillBehaviorConfig.generated.h"

class UMASkillBehavior;
class AMAProjectile_OverlapAOE;
class AMAProjectile_GroundTargetedAOE;
class AMAAbilityRangeActor;
class AMATargetActor_ChargeAtFwd;

USTRUCT(BlueprintType)
struct FSkillBehaviorConfigBase
{
	GENERATED_BODY()
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
	TSubclassOf<AMAProjectile_GroundTargetedAOE> DefaultProjectile;
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
	TSubclassOf<AMAProjectile_OverlapAOE> DefaultProjectile;
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
struct FConfig_Blink : public FSkillBehaviorConfigBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimMontage> MontageToPlay;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AMATargetActor_Movement> TargetActorClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxBlinkDistance = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMASkillVFXSet> VFXDataSet;
};

USTRUCT(BlueprintType)
struct FConfig_Dash : public FSkillBehaviorConfigBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimMontage> MontageToPlay;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ForwardLaunchForce = 500.f;
	float UpLaunchForce = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMASkillVFXSet> VFXDataSet;
};

USTRUCT(BlueprintType)
struct FConfig_Jump : public FSkillBehaviorConfigBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimMontage> MontageToPlay;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AMATargetActor_Movement> TargetActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxJumpDistance = 700.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinJumpDistance = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxJumpForce = 1000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinJumpForce = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VerticalLaunchForce = 400.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlamForce = -2000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMASkillVFXSet> VFXDataSet;
};

USTRUCT(BlueprintType)
struct FConfig_Rush : public FSkillBehaviorConfigBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimMontage> MontageToPlay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRushDuration = 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMASkillVFXSet> VFXDataSet;
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

USTRUCT(BlueprintType)
struct FSkillInformationDT : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UMAGameplayAbility> GrantedAbility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimMontage> SkillMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamageMultiplier = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseCooldownDuration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Ability.Behavior"))
	FGameplayTag DefaultBehaviorTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Ability.Attribute"))
	FGameplayTag DefaultElementalTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Ability.Utility"))
	FGameplayTag DefaultUtilityTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Ability.Cooldown"))
	FGameplayTag CooldownTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMASkillVFXSet> VFXDataSet;
};
