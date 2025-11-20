// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/InventoryItem.h"
#include "InventoryComponent.generated.h"

class UAbilitySystemComponent;
class UPA_ShopItem;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemAddedDelegate, const UInventoryItem* /*NewItem*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemRemovedDelegate, const FInventoryItemHandle& /*ItemHandle*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemStackCountChangeDelegate, const FInventoryItemHandle&, int /*NewCount*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnItemAbilityCommitted, const FInventoryItemHandle&, float /*CooldownDuration*/, float /*CooldownTimeRemaining*/);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();
	FOnItemAddedDelegate OnItemAdded;
	FOnItemRemovedDelegate OnItemRemoved;
	FOnItemStackCountChangeDelegate OnItemStackCountChanged;
	FOnItemAbilityCommitted OnItemAbilityCommitted;
	void TryActivateItem(const FInventoryItemHandle& ItemHandle);
	void TryPurchase(const UPA_ShopItem* ItemToPurchase);
	void SellItem(const FInventoryItemHandle& ItemHandle);
	float GetGold() const;
	FORCEINLINE int GetCapacity() const { return Capacity; }

	void ItemSlotChanged(const FInventoryItemHandle& Handle, int NewSlotNumber);
	UInventoryItem* GetInventoryItemByHandle(const FInventoryItemHandle& Handle) const;

	bool IsFullFor(const UPA_ShopItem* Item) const;

	bool IsAllSlotOccupied() const;
	UInventoryItem* GetAvaliableStackForItem(const UPA_ShopItem* Item) const;
	bool FoundIngredientForItem(const UPA_ShopItem* Item, TArray<UInventoryItem*>& OutIngredients);
	UInventoryItem* TryGetItemForShopItem(const UPA_ShopItem* Item) const;
	void TryActivateItemInSlot(int SlotNumber);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int Capacity = 6;

	UPROPERTY()
	UAbilitySystemComponent* OwnerAbilitySystemComponent;

	UPROPERTY()
	TMap<FInventoryItemHandle, UInventoryItem*> InventoryMap;

	void AbilityCommitted(class UGameplayAbility* CommittedAbility);

	/*********************************************************/
	/*                   Server                              */
	/*********************************************************/
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Purchase(const UPA_ShopItem* ItemToPurchase);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ActivateItem(FInventoryItemHandle ItemHandle);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SellItem(FInventoryItemHandle ItemHandle);
	void GrantItem(const UPA_ShopItem* NewItem);
	void ConsumeItem(UInventoryItem* Item);
	void RemoveItem(UInventoryItem* Item);
	void CheckItemCombination(const UInventoryItem* NewItem);

	/*********************************************************/
	/*                   Client                              */
	/*********************************************************/
private:
	UFUNCTION(Client, Reliable)
	void Client_ItemAdded(FInventoryItemHandle AssignedHandle, const UPA_ShopItem* Item);

	UFUNCTION(Client, Reliable)
	void Client_ItemRemoved(FInventoryItemHandle ItemHandle);

	UFUNCTION(Client, Reliable)
	void Client_ItemStackCountChanged(FInventoryItemHandle Handle, int NewCount);
};