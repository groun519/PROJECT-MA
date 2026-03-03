// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Engine/DataTable.h"
#include "GAS/Projectile/MAAbilityRangeActor.h"
#include "GAS/Projectile/MAProjectile.h"
#include "Inventory/MAItemTypes.h"
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
struct FSkillData : public FBaseItemData
{
	GENERATED_BODY()

public:
	FSkillData();
	/** 해당 스킬의 GA 블루프린트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="System")
	TSubclassOf<UMAGameplayAbility_Skill> GrantedAbility;
	/** 해당 스킬의 애니메이션 몽타주 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="System")
	UAnimMontage* SkillMontage;
	/** GA 블루프린트 안에 설정해 놓은 ID값과 동일하게 입력 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="System")
	FName SkillID;

	/** 이 스킬이 가진 특성 (근접공격 or 투사체 공격 or 타게팅 공격) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Ability.Trait"), Category="Traits")
	FGameplayTagContainer SkillTraits;
	
	/** 이 스킬의 초기 행동 모듈 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Module.Behavior"), Category="Module")
	FGameplayTag DefaultBehaviorTag;
	/** 이 스킬의 초기 속성 모듈 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Module.Elemental"), Category="Module")
	FGameplayTag DefaultElementalTag;
	/** 이 스킬의 초기 유틸리티 모듈 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Module.Utility"), Category="Module")
	FGameplayTag DefaultUtilityTag;
	/** 콤보 모듈 사용 시 설정할 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Module.Behavior.Combo"), Category="Module")
	FGameplayTag DefaultComboTag;

	/** 이 스킬이 어떤 액션을 취하는 지에 대한 태그 (특성과 비슷) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Ability.Action"), Category="Action Resource")
	FGameplayTagContainer ActionTags;
	/** 액션을 취하는데 필요한 데이터 (필요 없는 경우 생략 가능) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BaseStruct ="/Script/P_MA.SkillActionConfig"), Category="Action Resource")
	FInstancedStruct ActionData;

	
	/** 이 스킬의 데미지 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat")
	float BaseDamageMultiplier=1.f;
	/** 이 스킬의 쿨타임 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat")
	float BaseCooldown = 10.f;
	/** 이 스킬의 쿨타임 태그(스킬마다 별도로 설정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Ability.Cooldown"), Category="Stat")
	FGameplayTag CooldownTag;
	
	/** 출력할 스킬 이펙트 - (공격 몽타주에서 공격 범위 지정하는 경우에 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	TObjectPtr<UMASkillVFXSet> VFXDataSet;
	
	/** 스킬 사용 중 움직임이 가능한지 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Option")
	bool bCanMove=false;
	/** 스킬 사용 중 캐릭터 회전이 가능한지 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Option")
	bool bCanRotate=true;

	/** 스킬로 타격 시의 이펙트 (경직) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Effect.Reaction"), Category="Hit Reaction")
	FGameplayTag HitReactionTag;
	/** 경직의 힘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Hit Reaction")
	float ReactionForce = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hit Stop")
	bool bUseHitStop = false;
	/** 역경직 (묵직한 느낌 0.08 / 가벼운 느낌 0.02) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hit Stop", meta=(EditCondition="bUseHitStop"))
	float HitStopDuration = 0.05f;
	/** 역경직 (묵직한 느낌 0.0 / 가벼운 느낌 0.1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hit Stop", meta=(EditCondition="bUseHitStop"))
	float HitStopTimeDilation = 0.01f;
	/** 역경직 카메라 줌 효과 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hit Stop", meta=(EditCondition="bUseHitStop"))
	float HitStopZoomOffset = 10.f;
	/** 역경직 카메라 비네트 효과 세기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hit Stop", meta=(EditCondition="bUseHitStop"))
	float HitStopVignette = 1.f;

	
	/** UI용 Icon 항목 */
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	TSoftObjectPtr<UTexture2D> SkillIcon;
	/** UI용 스킬 이름 항목 */
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	FText DisplayName;
	/** UI용 스킬 설명 항목 */
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	FText Description;
	*/
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Ability.Trait"), Category="Requirement")
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
	/**최대 데미지까지의 충전 시간*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxChargeDuration = 3.f;
	/***/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxInputDelay = 3.4f;
};

USTRUCT(BlueprintType)
struct FBehavior_Combo : public FSkillBehaviorConfig
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* ComboMontage= nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> ComboModuleEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="State.Debuff"))
	FGameplayTag ComboHitReactionTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InputWindow = 3.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* ComboIcon = nullptr;
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