// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "MASkillTemplate.generated.h"

class AMATargetActor_Movement;
class UMASkillVFXSet;
class AMAProjectile_OverlapAOE;
class AMAProjectile_GroundTargetedAOE;
class UNiagaraSystem;
class AMAAbilityRangeActor;
class AGameplayAbilityTargetActor;
class UMASkillBehavior;

USTRUCT(BlueprintType)
struct FDefaultData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> MontageToPlay;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMASkillVFXSet> VFXDataSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)		float DamageMultiplier = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)		float CooldownDuration=0.f;
};

USTRUCT(BlueprintType)
struct FChainData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> MontageToPlay;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMASkillVFXSet> VFXDataSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		TMap<FName, float> ComboDamageMultipliers;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)		float CooldownDuration=0.f;
};

USTRUCT(BlueprintType)
struct FHoldData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> MontageToPlay;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMASkillVFXSet> VFXDataSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)		float DamageMultiplier = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)		float CooldownDuration=0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MaxHoldDuration = 2.5f;
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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> MontageToPlay;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMASkillVFXSet> VFXDataSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)		float CooldownDuration=0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MaxChargeDuration = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float TimeoutDuration = 4.5f;
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
	TSubclassOf<AGameplayAbilityTargetActor> TargetActorClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMAAbilityRangeActor> RangeActorClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MaxDistance = 1000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MinRadius = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MaxRadius = 400.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float DefaultVFXRadius = 200.f;
};

USTRUCT(BlueprintType)
struct FSpawnAtFwdData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> MontageToPlay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMAProjectile_OverlapAOE> DefaultProjectile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, TSubclassOf<AMAProjectile_OverlapAOE>> ElementalProjectiles;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)		float DamageMultiplier = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)		float CooldownDuration=0.f;
	
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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> MontageToPlay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMAProjectile_GroundTargetedAOE> DefaultProjectile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, TSubclassOf<AMAProjectile_GroundTargetedAOE>> ElementalProjectiles;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AGameplayAbilityTargetActor> TargetActorClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMAAbilityRangeActor> RangeActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)		float DamageMultiplier = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)		float CooldownDuration=0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MaxDistance = 1000.f;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)		float AbilityRange = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float TravelTime = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float SpawnHeight = 700.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		int32 ProjectileCount = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float SpawnDelay = 0.1f;
};

USTRUCT(BlueprintType)
struct FMovement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> MontageToPlay;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMASkillVFXSet> VFXDataSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)		float DamageMultiplier = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)		float CooldownDuration=0.f;
};

USTRUCT(BlueprintType)
struct FMovement_Blink : public FMovement
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMATargetActor_Movement> TargetActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MaxBlinkDistance = 500.f;
};

USTRUCT(BlueprintType)
struct FMovement_Dash : public FMovement
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float UpLaunchForce = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float ForwardLaunchForce = 2000.f;
};

USTRUCT(BlueprintType)
struct FMovement_Jump : public FMovement
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMATargetActor_Movement> TargetActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MaxJumpDistance = 700.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MinJumpDistance = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MaxJumpForce = 1000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MinJumpForce = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float VerticalLaunchForce = 400.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float SlamForce = -2000.f;
};

USTRUCT(BlueprintType)
struct FMovement_Rush : public FMovement
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		float MaxRushDuration = 3.f;
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
	TSubclassOf<UGameplayEffect> DamageGEClass;
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
	FGameplayTag ElementModuleTag;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Setup")
	FGameplayTag UtilityModuleTag;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Setup")
	FGameplayTag BehaviorModuleTag;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Setup")
	FGameplayTag CooldownTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module | Default")
	FDefaultData DefaultData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module | Chain")
	FChainData ChainData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module | Charge")
	FChargeData ChargeData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module | Hold")
	FHoldData HoldData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module | HoldLaunch")
	FHoldLaunchData HoldLaunchData;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module | SpawnFwd")
	FSpawnAtFwdData SpawnAtFwdData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module | SpawnTarget")
	FSpawnAtTargetData SpawnAtTargetData;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module | ChargeTarget")
	FChargeTargetData ChargeTargetData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module | ChargeFwd")
	FChargeFwdData ChargeFwdData;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement | Blink")
	FMovement_Blink BlinkData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement | Dash")
	FMovement_Dash DashData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement | Rush")
	FMovement_Rush RushData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement | Jump")
	FMovement_Jump JumpData;
};
