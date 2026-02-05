// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Engine/DataTable.h"
#include "GAS/Projectile/MAAbilityRangeActor.h"
#include "GAS/Projectile/MAProjectile.h"
#include "StructUtils/Public/InstancedStruct.h"
#include "MASkillModuleData.generated.h"

class UMAProjectileSkinData;
class UNiagaraSystem;
class UMASkillVFXSet;
class UMAGameplayAbility_Skill;
class UGameplayEffect;
class UMASkillModule;

/*
 *	모든 스킬 관리 데이터 테이블
 */
USTRUCT(BlueprintType)
struct FSkillData : public FTableRowBase
{
	GENERATED_BODY()

public:
	FSkillData();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resource")
	TSubclassOf<UMAGameplayAbility_Skill> AbilityClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resource")
	UAnimMontage* SkillMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resource")
	UTexture2D* SkillIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Skill.Trait"), Category="Traits")
	FGameplayTagContainer SkillTraits;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Module.Behavior"), Category="Module")
	FGameplayTag DefaultBehaviorTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Module.Elemental"), Category="Module")
	FGameplayTag DefaultElementalTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Module.Utility"), Category="Module")
	FGameplayTag DefaultUtilityTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Ability.Action"), Category="Action Resource")
	FGameplayTagContainer ActionTags;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Action Resource", meta=(BaseStruct ="/Script/P_MA.SkillActionConfig"))
	FInstancedStruct ActionData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Ability.Cooldown"), Category="Skill Stat")
	FGameplayTag CooldownTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Stat")
	float BaseCooldown = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Stat")
	float BaseDamageMultiplier=1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	TObjectPtr<UMASkillVFXSet> VFXDataSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Option")
	bool bCanMove=false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Option")
	bool bCanRotate=true;
};

// 행동 모듈 데이터 테이블
USTRUCT(BlueprintType)
struct FModuleBehaviorData : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UMASkillModule> ModuleClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BaseStruct = "/Script/P_MA.SkillBehaviorConfig"))
	FInstancedStruct ModuleConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Skill.Trait"), Category="Requirement")
	FGameplayTagContainer RequiredTraits;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Override")
	//bool bReplaceActionTags = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Ability.Action"), Category="Override")
	FGameplayTagContainer ActionTagOverride;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Override", meta=(BaseStruct = "/Script/P_MA.SkillActionConfig"))
	FInstancedStruct ActionDataOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Information")
	FText Description;
};

// 행동 모듈 데이터
USTRUCT(BlueprintType)
struct FSkillBehaviorConfig
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FBehavior_Hold : public FSkillBehaviorConfig
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HoldingDamageMultiplier = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHoldDuration = 2.5f;
};

USTRUCT(BlueprintType)
struct FBehavior_Charge : public FSkillBehaviorConfig
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxChargeDuration = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxInputDelay = 3.4f;
};


// 속성 모듈 데이터 테이블
USTRUCT(BlueprintType)
struct FModuleElementalData : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Module.Elemental"))
	FGameplayTag ElementalTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> AdditionalEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor EffectColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier=1.f;
};


// 유틸리티 모듈 데이터 테이블
USTRUCT(BlueprintType)
struct FModuleUtilityData : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier=1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MontagePlayRate=1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownMultiplier=1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="CooldownMultiplier ==0.0"))
	float ChanceToReset=0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> BuffGEOnActive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> BuffGEOnEnd;
	
	UPROPERTY(EditAnywhere)
	FText Description;
};

// Action 데이터 구조체 [투사체 or 타게팅]
USTRUCT(BlueprintType)
struct FSkillActionConfig
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FActionConfig_Projectile : public FSkillActionConfig
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMAProjectileSkinData> SkinData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumOfProjectiles = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplierPerProjectile = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExplodeRadius = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPenetrating = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRadial = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="!bIsRadial"))
	float SpreadAngle = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnDistanceFromCharacter = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AngleOffset = 0.f;
};

USTRUCT(BlueprintType)
struct FActionConfig_Targeting : public FSkillActionConfig
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AGameplayAbilityTargetActor> TargetActorClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMAAbilityRangeActor> RangeActorClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMAProjectileSkinData> SkinData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumOfProjectiles = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplierPerProjectile = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnHeight = 600.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExplodeRadius = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistance = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 700.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Charge Box")
	float SkillWidth = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Charge Box")
	float DecalDepth = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Charge Circle")
	float MinSize = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Charge Circle")
	float MaxSize = 500.f;
};