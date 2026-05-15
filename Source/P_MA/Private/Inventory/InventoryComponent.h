#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/InventoryItem.h"
#include "InventoryComponent.generated.h"

class UAbilitySystemComponent;
class UDataTable;

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
	
	void TryActivateItem(const FInventoryItemHandle& ItemHandle);
	void SellItem(const FInventoryItemHandle& ItemHandle);
	
	void TryPurchaseItem(FName ItemRowName, UDataTable* SourceTable);
	
	float GetGold() const;
	FORCEINLINE int GetCapacity() const { return Capacity; }

	void ItemSlotChanged(const FInventoryItemHandle& Handle, int NewSlotNumber);
	UInventoryItem* GetInventoryItemByHandle(const FInventoryItemHandle& Handle) const;
	
	bool IsFullFor(FName ItemRowName, UDataTable* SourceTable) const;
	bool IsAllSlotOccupied() const;
	
	UInventoryItem* GetAvaliableStackFor(FName ItemRowName, UDataTable* SourceTable) const;

	void TryActivateItemInSlot(int SlotNumber);

protected:
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
	/* Server                              */
	/*********************************************************/
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_PurchaseItem(FName ItemRowName, UDataTable* SourceTable);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ActivateItem(FInventoryItemHandle ItemHandle);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SellItem(FInventoryItemHandle ItemHandle);
	
	void GrantItem(FName ItemRowName, UDataTable* SourceTable);

	void ConsumeItem(UInventoryItem* Item);
	void RemoveItem(UInventoryItem* Item);
	

	/*********************************************************/
	/* Client                              */
	/*********************************************************/
private:
	UFUNCTION(Client, Reliable)
	void Client_ItemAdded(FInventoryItemHandle AssignedHandle, FName ItemRowName, UDataTable* SourceTable);

	UFUNCTION(Client, Reliable)
	void Client_ItemRemoved(FInventoryItemHandle ItemHandle);

	UFUNCTION(Client, Reliable)
	void Client_ItemStackCountChanged(FInventoryItemHandle Handle, int NewCount);
};
