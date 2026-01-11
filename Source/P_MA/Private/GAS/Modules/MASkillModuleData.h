// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Engine/DataTable.h"
#include "StructUtils/Public/InstancedStruct.h"
#include "MASkillModuleData.generated.h"

class UMASkillVFXSet;
class UMAGameplayAbility_Skill;
class UGameplayEffect;
class UMASkillModule;

USTRUCT(BlueprintType)
struct FSkillData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resource")
	TSubclassOf<UMAGameplayAbility_Skill> AbilityClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resource")
	UAnimMontage* SkillMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resource")
	UTexture2D* SkillIcon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Module.Behavior"), Category="Module")
	FGameplayTag DefaultBehaviorTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Module.Elemental"), Category="Module")
	FGameplayTag DefaultElementalTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Module.Utility"), Category="Module")
	FGameplayTag DefaultUtilityTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Ability.Action"), Category="Action Resource")
	FGameplayTagContainer ActionTags;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Action Resource")
	TSubclassOf<AActor> ProjectileClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Action Resource")
	TSubclassOf<AActor> ChargeActorClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Action Resource")
	TSubclassOf<AGameplayAbilityTargetActor> TargetActorClass;
	
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

USTRUCT(BlueprintType)
struct FModuleBehaviorData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UMASkillModule> ModuleClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BaseStruct = "SkillBehaviorConfig"))
	FInstancedStruct ModuleConfig;
};

USTRUCT(BlueprintType)
struct FModuleElementalData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> AdditionalEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor EffectColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier=1.f;
};


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

