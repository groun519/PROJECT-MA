// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilitySpecHandle.h"
#include "Inventory/MAItemTypes.h" // [필수] 아까 만든 헤더 포함
#include "InventoryItem.generated.h"

class UAbilitySystemComponent;
// class UPA_ShopItem; // [삭제] 더 이상 필요 없음

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAbilityCanCastUpdatedDelegate, bool /*bCanCast*/)

USTRUCT()
struct FInventoryItemHandle
{
	GENERATED_BODY()
public:
	FInventoryItemHandle();
	static FInventoryItemHandle InvalidHandle();
	static FInventoryItemHandle CreateHandle();

	bool IsValid() const;
	uint32 GetHandleId() const { return HandleId; }
private:
	explicit FInventoryItemHandle(uint32 Id);
	UPROPERTY()
	uint32 HandleId;
	static uint32 GenerateNextId();
	static uint32 GetInvalidId();
};

FORCEINLINE bool operator==(const FInventoryItemHandle& Lhs, const FInventoryItemHandle& Rhs)
{
	return Lhs.GetHandleId() == Rhs.GetHandleId();
}

FORCEINLINE uint32 GetTypeHash(const FInventoryItemHandle& Key)
{
	return Key.GetHandleId();
}

bool operator==(const FInventoryItemHandle& Lhs, const FInventoryItemHandle& Rhs);
uint32 GetTypeHash(const FInventoryItemHandle& Key);

/**
 * 런타임 인벤토리 아이템 객체
 * (이제 DataAsset 대신 DataTable의 Row를 참조합니다)
 */
UCLASS()
class UInventoryItem : public UObject
{
	GENERATED_BODY()

public:
	UInventoryItem();

	FOnAbilityCanCastUpdatedDelegate OnAbilityCanCastUpdated;

	bool IsSameItem(FName OtherRowName, UDataTable* OtherTable) const;

	// [변경] 초기화 함수 파라미터 수정 (ShopItem -> RowName & DataTable)
	void InitItem(const FInventoryItemHandle& NewHandle, FName NewRowName, UDataTable* InSourceTable, UAbilitySystemComponent* AbilitySystemComponent);

	// --- [추가] 데이터 접근 헬퍼 함수들 ---
	const FBaseItemData* GetBaseData() const;           // 공통 데이터(이름, 아이콘)
	const FConsumableItemData* GetConsumableData() const; // 소비 아이템 데이터
	const FEquipmentItemData* GetEquipmentData() const;   // 장비 아이템 데이터
	const FSkillItemData* GetSkillData() const;           // 스킬 아이템 데이터
	
	// 기존 코드를 덜 고치기 위한 어댑터 함수들 (기존 Getter 대체)
	UTexture2D* GetIcon() const;
	TSubclassOf<class UGameplayAbility> GetGrantedAbility() const;
	bool IsStackable() const;
	int32 GetMaxStackCount() const;
	// -------------------------------------

	bool AddStackCount();
	bool ReduceStackCount();
	bool SetStackCount(int NewStackCount);
	bool IsStackFull() const;

	// bool IsForItem(const UPA_ShopItem* Item) const; // [삭제] 파라미터 타입이 바뀌어서 로직 수정 필요 (나중에 구현)
	
	bool IsGrantintAbility(TSubclassOf<class UGameplayAbility> AbilityClass) const;
	bool IsGrantingAnyAbility() const;
	
	bool IsValid() const;
	FInventoryItemHandle GetHandle() const { return Handle; }

	bool TryActivateGrantedAbility();
	void ApplyConsumeEffect();
	void RemoveGASModifications();
	
	FORCEINLINE int GetStackCount() const { return StackCount; }
	void SetSlot(int NewSlot);
	int GetItemSlot() const { return Slot; }

	float GetAbilityCooldownTimeRemaining() const;
	float GetAbilityCooldownDuration() const;
	bool CanCastAbility() const;
	
	FGameplayAbilitySpecHandle GetGrantedAbilitySpecHandle() const { return GrantedAbiltiySpecHandle; }
	void SetGrantedAbilitySpecHandle(FGameplayAbilitySpecHandle SpecHandle) { GrantedAbiltiySpecHandle = SpecHandle; }
	
private:
	void ApplyGASModifications();

	UAbilitySystemComponent* OwnerAbilitySystemComponent;

	// [변경] 원본 데이터 소스 변경
	// const UPA_ShopItem* ShopItem; // [삭제]
	
	UPROPERTY()
	FName ItemRowName; // 테이블 행 이름

	UPROPERTY()
	TObjectPtr<UDataTable> SourceDataTable; // 연결된 데이터 테이블

	UPROPERTY()
	EMAItemType CachedType = EMAItemType::None; // 타입 캐싱 (매번 검색 방지)

	FInventoryItemHandle Handle;
	int StackCount;
	int Slot;

	FActiveGameplayEffectHandle AppliedEquipedEffectHandle;
	FGameplayAbilitySpecHandle GrantedAbiltiySpecHandle;
};