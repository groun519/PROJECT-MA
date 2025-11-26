// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "MASkillTemplate.generated.h"

class UNiagaraSystem;
class AMAAbilityRangeActor;
class AGameplayAbilityTargetActor;
class UMASkillBehavior;

USTRUCT(BlueprintType)
struct FChainData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> ComboDamageMultipliers;
};

USTRUCT(BlueprintType)
struct FHoldData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MaxHoldDuration = 2.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float ShortCoolDownDuration = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		bool bCanMove = false;
};

USTRUCT(BlueprintType)
struct FHoldLaunchData : public FHoldData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float FirstLaunchForce = 1000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float OtherLaunchForce = 400.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float SmashForce = 1400.f;
};

USTRUCT(BlueprintType)
struct FChargeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MaxChargeDuration = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float TimeoutDuration = 4.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float ShortCoolDownDuration = 1.0f;
};

USTRUCT(BlueprintType)
struct FChargeFwdData : public FChargeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AGameplayAbilityTargetActor> TargetActorClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MinDistance = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MaxDistance = 1000.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float SkillWidth = 96.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float DefaultVFXLength = 1000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float DefaultVFXWidth = 120.f;
};

USTRUCT(BlueprintType)
struct FChargeTargetData : public FChargeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMAAbilityRangeActor> RangeActorClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AGameplayAbilityTargetActor> TargetActorClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MaxDistance = 1000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MinRadius = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MaxRadius = 400.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float DefaultVFXRadius = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UNiagaraSystem> ExecutionVFX;
};

USTRUCT(BlueprintType)
struct FSpawnAtFwdData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> DefaultProjectile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, TSubclassOf<AActor>> ElementalProjectiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float ProjectileSpeed = 700.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MaxDistance = 2000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float ExplodeRadius = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		FName MuzzleSocketName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		int32 ProjectileCount = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float SpawnDelay = 0.1f;
};

USTRUCT(BlueprintType)
struct FSpawnAtTargetData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AGameplayAbilityTargetActor> TargetActorClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMAAbilityRangeActor> RangeActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> DefaultProjectile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, TSubclassOf<AActor>> ElementalProjectiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float TravelTime = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MaxDistance = 1000.f;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)		float AbilityRange = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float SpawnHeight = 700.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		int32 ProjectileCount = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float SpawnDelay = 0.1f;
};

/**
 * 
 */
UCLASS()
class UMASkillTemplate : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category="Modules")
	TMap<FGameplayTag, TSubclassOf<UMASkillBehavior>> BehaviorModuleMap;
	
	UPROPERTY(EditDefaultsOnly, Category="Common")
	TSubclassOf<UGameplayEffect> CooldownGEClass;
};


USTRUCT(BlueprintType)
struct FSkillDefinitionDT : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Setup")
	TObjectPtr<UMASkillTemplate> SkillTemplate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Setup")
	FGameplayTag BehaviorModuleTag;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Setup")
	FGameplayTag CooldownTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Common")
	TObjectPtr<UAnimMontage> MontageToPlay;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Common")
	float DamageMultiplier = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Common")
	float CooldownDuration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module | Chain")
	FChainData ChainData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module | Hold")
	FHoldData HoldData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module | HoldLaunch")
	FHoldLaunchData HoldLaunchData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module | Charge")
	FChargeData ChargeData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module | ChargeTarget")
	FChargeTargetData ChargeTargetData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module | ChargeFwd")
	FChargeFwdData ChargeFwdData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module | SpawnFwd")
	FSpawnAtFwdData SpawnAtFwdData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module | SpawnTarget")
	FSpawnAtTargetData SpawnAtTargetData;
	
	
};
