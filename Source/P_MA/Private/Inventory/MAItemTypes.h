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
 * 아이템 타입을 구분하기 위한 Enum
 * (런타임에서 이 아이템이 장비인지 소비인지 빠르게 알기 위해 사용)
 */
UENUM(BlueprintType)
enum class EMAItemType : uint8
{
	None,
	Consumable UMETA(DisplayName = "Consumable"), // 소비 (포션 등)
	Equipment  UMETA(DisplayName = "Equipment"),  // 장비 (무기 등)
	Skill      UMETA(DisplayName = "Skill"),      // 스킬 (스킬북)
	Etc        UMETA(DisplayName = "Etc")
};

/**
 * [부모] 모든 아이템의 공통 데이터 (아이콘, 이름, 가격 등)
 */
USTRUCT(BlueprintType)
struct FBaseItemData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
	EMAItemType ItemType = EMAItemType::None; // 반드시 설정해야 함!

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
 * [자식 1] 소비 아이템 데이터 (스택 O, 소비 이펙트 O)
 */
USTRUCT(BlueprintType)
struct FConsumableItemData : public FBaseItemData
{
	GENERATED_BODY()

public:
	FConsumableItemData()
	{
		ItemType = EMAItemType::Consumable; // 기본값 설정
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable")
	int32 MaxStackCount = 99;

	// 소비했을 때 발동할 GE (체력 회복 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable")
	TSubclassOf<UGameplayEffect> ConsumeEffect; 
};

/**
 * [자식 2] 장비 아이템 데이터 (스택 X, 장착 이펙트 O, 슬롯 태그 O)
 */
USTRUCT(BlueprintType)
struct FEquipmentItemData : public FBaseItemData
{
	GENERATED_BODY()

public:
	FEquipmentItemData()
	{
		ItemType = EMAItemType::Equipment;
	}
	
	// 장비는 어디에 낄지(슬롯) 정보가 중요함
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FGameplayTag EquipSlotTag; 

	// 장착 시 적용할 GE (공격력 증가 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TSubclassOf<UGameplayEffect> EquipEffect; 
};

/**
 * [자식 3] 스킬 아이템 데이터 (스택 X, 습득할 어빌리티 O)
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

	// 이 아이템을 사면 배우게 될 어빌리티 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TSubclassOf<UGameplayAbility> GrantedAbility; 
    
    // 필요하다면 쿨타임 정보 등 추가 (툴팁용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float CooldownDuration = 0.0f;
};