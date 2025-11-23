// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/InventoryItem.h"
#include "Widget/MAAbilityGauge.h" // FAbilityWidgetData 등
#include "Inventory/MAItemTypes.h" // [필수] 구조체 정의 포함
#include "InventoryComponent.generated.h"

class UAbilitySystemComponent;
class UDataTable;

// 델리게이트 파라미터 수정
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemAddedDelegate, const UInventoryItem* /*NewItem*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemRemovedDelegate, const FInventoryItemHandle& /*ItemHandle*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemStackCountChangeDelegate, const FInventoryItemHandle&, int /*NewCount*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnItemAbilityCommitted, const FInventoryItemHandle&, float /*CooldownDuration*/, float /*CooldownTimeRemaining*/);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();

	FOnItemAddedDelegate OnItemAdded;
	FOnItemRemovedDelegate OnItemRemoved;
	FOnItemStackCountChangeDelegate OnItemStackCountChanged;
	FOnItemAbilityCommitted OnItemAbilityCommitted;

	// 아이템 사용/판매
	void TryActivateItem(const FInventoryItemHandle& ItemHandle);
	void SellItem(const FInventoryItemHandle& ItemHandle);

	// [변경] 구매 요청 (DataAsset* -> RowName, Table)
	void TryPurchaseItem(FName ItemRowName, UDataTable* SourceTable);

	// [변경] 스킬 구매 요청 (UI에서 호출)
	void TryPurchaseSkill(FName SkillRowName, UDataTable* SourceTable);

	float GetGold() const;
	FORCEINLINE int GetCapacity() const { return Capacity; }

	void ItemSlotChanged(const FInventoryItemHandle& Handle, int NewSlotNumber);
	UInventoryItem* GetInventoryItemByHandle(const FInventoryItemHandle& Handle) const;

	// [변경] 인벤토리가 꽉 찼는지 확인
	bool IsFullFor(FName ItemRowName, UDataTable* SourceTable) const;
	bool IsAllSlotOccupied() const;

	// [변경] 스택 가능한 슬롯 찾기
	UInventoryItem* GetAvaliableStackFor(FName ItemRowName, UDataTable* SourceTable) const;

	void TryActivateItemInSlot(int SlotNumber);

protected:
	virtual void BeginPlay() override;

	// [변경] 스킬 구매 (서버)
	UFUNCTION(Server, Reliable, WithValidation) 
	void Server_PurchaseSkill(FName SkillRowName, UDataTable* SourceTable);

private:	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int Capacity = 6;

	UPROPERTY()
	UAbilitySystemComponent* OwnerAbilitySystemComponent;

	UPROPERTY()
	TMap<FInventoryItemHandle, UInventoryItem*> InventoryMap;

	void AbilityCommitted(class UGameplayAbility* CommittedAbility);

	/*********************************************************/
	/* Server                              */
	/*********************************************************/
	
	// [변경] 아이템 구매 (서버)
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_PurchaseItem(FName ItemRowName, UDataTable* SourceTable);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ActivateItem(FInventoryItemHandle ItemHandle);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SellItem(FInventoryItemHandle ItemHandle);

	// [변경] 아이템 지급 (내부 함수)
	void GrantItem(FName ItemRowName, UDataTable* SourceTable);

	void ConsumeItem(UInventoryItem* Item);
	void RemoveItem(UInventoryItem* Item);
	
	// [보류] 조합 기능은 일단 주석 처리하겠습니다 (복잡도 감소)
	// void CheckItemCombination(const UInventoryItem* NewItem);

	/*********************************************************/
	/* Client                              */
	/*********************************************************/
private:
	// [변경] 클라이언트에게 아이템 추가 알림
	UFUNCTION(Client, Reliable)
	void Client_ItemAdded(FInventoryItemHandle AssignedHandle, FName ItemRowName, UDataTable* SourceTable);

	UFUNCTION(Client, Reliable)
	void Client_ItemRemoved(FInventoryItemHandle ItemHandle);

	UFUNCTION(Client, Reliable)
	void Client_ItemStackCountChanged(FInventoryItemHandle Handle, int NewCount);
};