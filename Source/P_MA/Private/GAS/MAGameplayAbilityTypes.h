// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MAGameplayAbilityTypes.generated.h"

struct FMAGameplayEffectContext;

USTRUCT()
struct FMAGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	bool IsCriticalHit() const {return bIsCriticalHit;}
	void SetIsCriticalHit(bool bInIsCriticalHit) {bIsCriticalHit = bInIsCriticalHit;}
	virtual UScriptStruct* GetScriptStruct() const override {return StaticStruct();}
	virtual FMAGameplayEffectContext* Duplicate() const override
	{
		FMAGameplayEffectContext* NewContext = new FMAGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;
protected:
	UPROPERTY()
	bool bIsCriticalHit = false;
};

template<>
struct TStructOpsTypeTraits<FMAGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FMAGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

UENUM(BlueprintType)
enum class EMAAbilityInputID : uint8
{
	None				UMETA(DisplayName = "None"),

	Attack				UMETA(DisplayName = "Attack"),
	Skill1				UMETA(DisplayName = "Skill1"),
	Skill2				UMETA(DisplayName = "Skill2"),
	Skill3				UMETA(DisplayName = "Skill3"),
	Skill4				UMETA(DisplayName = "Skill4"),
	Ultimate			UMETA(DisplayName = "Ultimate"),
	
	Movement			UMETA(DisplayName = "Movement"),

	Confirm				UMETA(DisplayName = "Confirm"),
	Cancel				UMETA(DisplayName = "Cancel"),
};

USTRUCT(BlueprintType)
struct FGenericDamageEffectDef
{
	GENERATED_BODY()

public:
	FGenericDamageEffectDef();
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere)
	FVector PushVelocity;
};

USTRUCT(BlueprintType)
struct FPlayerBaseStats : public FTableRowBase
{
	GENERATED_BODY()
FPlayerBaseStats();
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> Class;

	UPROPERTY(EditAnywhere)
	float BaseMaxHealth;

	UPROPERTY(EditAnywhere)
	float BaseAttack;
	
	UPROPERTY(EditAnywhere)
	float BaseDamageVariance;

	UPROPERTY(EditAnywhere)
	float BaseAttackSpeed;
	
	UPROPERTY(EditAnywhere)
	float BaseCriticalChance;

	UPROPERTY(EditAnywhere)
	float BaseCriticalDamage;
	
	UPROPERTY(EditAnywhere)
	float BaseAttackRange;

	UPROPERTY(EditAnywhere)
	float BaseMoveSpeed;
	
	UPROPERTY(EditAnywhere)
	float BaseArmor;

	UPROPERTY(EditAnywhere)
	float BaseArmorPenetration;
	
	UPROPERTY(EditAnywhere)
	float BaseGold;
};

USTRUCT(BlueprintType)
struct FMonsterBaseStats : public FTableRowBase
{
	GENERATED_BODY()
	FMonsterBaseStats();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> Class;

	UPROPERTY(EditAnywhere)
	float BaseMaxHealth;

	UPROPERTY(EditAnywhere)
	float BaseAttack;
	
	UPROPERTY(EditAnywhere)
	float BaseDamageVariance;

	UPROPERTY(EditAnywhere)
	float BaseMoveSpeed;

	UPROPERTY(EditAnywhere)
	float BaseAttackSpeed;

	UPROPERTY(EditAnywhere)
	float BaseArmor;

	UPROPERTY(EditAnywhere)
	float BaseArmorPenetration;

	UPROPERTY(EditAnywhere)
	float BaseFuryMax;
	
	UPROPERTY(EditAnywhere, meta=(Categories="Stats.Immunity"))
	FGameplayTagContainer BaseImmunityTags;
};