// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MAItemTypes.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UTexture2D;
class USkeletalMesh;

/**
 * 
 * 
 */
UENUM(BlueprintType)
enum class EMAItemType : uint8
{
	None,
	Consumable UMETA(DisplayName = "Consumable"),
	Equipment  UMETA(DisplayName = "Equipment"),  
	Skill      UMETA(DisplayName = "Skill"),     
	Etc        UMETA(DisplayName = "Etc")
};

/**
 * 
 */
USTRUCT(BlueprintType)
struct FBaseItemData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
	EMAItemType ItemType = EMAItemType::None; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
	float Price = 0.0f;
};

/**
 * 
 */
USTRUCT(BlueprintType)
struct FConsumableItemData : public FBaseItemData
{
	GENERATED_BODY()

public:
	FConsumableItemData()
	{
		ItemType = EMAItemType::Consumable;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable")
	int32 MaxStackCount = 99;
	
	// [변경] 단일 변수 -> 배열 (Array)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable")
	TArray<TSubclassOf<UGameplayEffect>> ConsumeEffects; 
};

/**
 * */
USTRUCT(BlueprintType)
struct FEquipmentItemData : public FBaseItemData
{
	GENERATED_BODY()

public:
	FEquipmentItemData()
	{
		ItemType = EMAItemType::Equipment;
	}
	
	// [변경] 단일 변수 -> 배열 (Array)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TArray<TSubclassOf<UGameplayEffect>> EquipEffects; 
};

/**
 * 
 */
USTRUCT(BlueprintType)
struct FSkillItemData : public FBaseItemData
{
	GENERATED_BODY()

public:
	FSkillItemData()
	{
		ItemType = EMAItemType::Skill;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TSubclassOf<UGameplayAbility> GrantedAbility; 
	
};