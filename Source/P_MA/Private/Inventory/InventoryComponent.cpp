// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Inventory/SkillBookComponent.h" 
#include "Player/MAPlayerCharacter.h"
#include "Framework/MAAssetManager.h"
#include "GAS/MAPlayerAttributeSet.h"
#include "Inventory/PA_ShopItem.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UInventoryComponent::TryActivateItem(const FInventoryItemHandle& ItemHandle)
{
	UInventoryItem* InventoryItem = GetInventoryItemByHandle(ItemHandle);
	if (!InventoryItem)
		return;

	Server_ActivateItem(ItemHandle);
}

void UInventoryComponent::TryPurchase(const UPA_ShopItem* ItemToPurchase)
{
	if (!OwnerAbilitySystemComponent)
		return;

	Server_Purchase(ItemToPurchase);
}

void UInventoryComponent::SellItem(const FInventoryItemHandle& ItemHandle)
{
	Server_SellItem(ItemHandle);
}

float UInventoryComponent::GetGold() const
{
	bool bFound = false;
	if (OwnerAbilitySystemComponent)
	{
		float Gold = OwnerAbilitySystemComponent->GetGameplayAttributeValue(UMAPlayerAttributeSet::GetGoldAttribute(), bFound);
		if (bFound)
		{
			return Gold;
		}
	}

	return 0.f;
}

void UInventoryComponent::ItemSlotChanged(const FInventoryItemHandle& Handle, int NewSlotNumber)
{
	if (UInventoryItem* FoundItem = GetInventoryItemByHandle(Handle))
	{
		FoundItem->SetSlot(NewSlotNumber);
	}
}

UInventoryItem* UInventoryComponent::GetInventoryItemByHandle(const FInventoryItemHandle& Handle) const
{
	UInventoryItem* const* FoundItem = InventoryMap.Find(Handle);
	if (FoundItem)
	{
		return *FoundItem;
	}

	return nullptr;
}

bool UInventoryComponent::IsFullFor(const UPA_ShopItem* Item) const
{
	if (!Item)
		return false;

	if (IsAllSlotOccupied())
	{
		return GetAvaliableStackForItem(Item) == nullptr;
	}

	return false;
}

bool UInventoryComponent::IsAllSlotOccupied() const
{
	return InventoryMap.Num() >= GetCapacity();
}

UInventoryItem* UInventoryComponent::GetAvaliableStackForItem(const UPA_ShopItem* Item) const
{
	if (!Item->GetIsStackable())
		return nullptr;

	for (const TPair<FInventoryItemHandle, UInventoryItem*>& ItemPair : InventoryMap)
	{
		if (ItemPair.Value && ItemPair.Value->IsForItem(Item) && !ItemPair.Value->IsStackFull())
		{
			return ItemPair.Value;
		}
	}

	return nullptr;
}

bool UInventoryComponent::FoundIngredientForItem(const UPA_ShopItem* Item, TArray<UInventoryItem*>& OutIngredients)
{
	const FItemCollection* Ingredients = UMAAssetManager::Get().GetIngredientForItem(Item);
	if (!Ingredients)
		return false;

	bool bAllFound = true;
	for (const UPA_ShopItem* Ingredient : Ingredients->GetItems())
	{
		UInventoryItem* FoundItem = TryGetItemForShopItem(Ingredient);
		if (!FoundItem)
		{
			bAllFound = false;
			break;
		}

		OutIngredients.Add(FoundItem);
	}

	return bAllFound;
}

UInventoryItem* UInventoryComponent::TryGetItemForShopItem(const UPA_ShopItem* Item) const
{
	if (!Item)
		return nullptr;

	for (const TPair<FInventoryItemHandle, UInventoryItem*>& ItemHandlePair : InventoryMap)
	{
		if (ItemHandlePair.Value && ItemHandlePair.Value->GetShopItem() == Item)
		{
			return ItemHandlePair.Value;
		}
	}

	return nullptr;
}

void UInventoryComponent::TryActivateItemInSlot(int SlotNumber)
{
	for (TPair<FInventoryItemHandle, UInventoryItem*>& ItemPair : InventoryMap)
	{
		if (ItemPair.Value->GetItemSlot() == SlotNumber)
		{
			Server_ActivateItem(ItemPair.Key);
			return;
		}
	}
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (OwnerAbilitySystemComponent)
		OwnerAbilitySystemComponent->AbilityCommittedCallbacks.AddUObject(this, &UInventoryComponent::AbilityCommitted);
}

void UInventoryComponent::AbilityCommitted(UGameplayAbility* CommittedAbility)
{
	if (!CommittedAbility)
		return;

	float CooldownTimeRemaining = 0.f;
	float CooldownDuration = 0.f;

	CommittedAbility->GetCooldownTimeRemainingAndDuration(
		CommittedAbility->GetCurrentAbilitySpecHandle(),
		CommittedAbility->GetCurrentActorInfo(),
		CooldownTimeRemaining,
		CooldownDuration
	);

	for (TPair<FInventoryItemHandle, UInventoryItem*>& ItemPair : InventoryMap)
	{
		if (!ItemPair.Value)
			continue;

		if (ItemPair.Value->IsGrantintAbility(CommittedAbility->GetClass()))
		{
			OnItemAbilityCommitted.Broadcast(ItemPair.Key, CooldownDuration, CooldownTimeRemaining);
		}
	}
}

void UInventoryComponent::Server_ActivateItem_Implementation(FInventoryItemHandle ItemHandle)
{
	UInventoryItem* InventoryItem = GetInventoryItemByHandle(ItemHandle);
	if (!InventoryItem)
		return;

	InventoryItem->TryActivateGrantedAbility();
	const UPA_ShopItem* Item = InventoryItem->GetShopItem();
	if (Item->GetIsConsumable())
	{
		ConsumeItem(InventoryItem);
	}
}

bool UInventoryComponent::Server_ActivateItem_Validate(FInventoryItemHandle ItemHandle)
{
	return true;
}

void UInventoryComponent::Server_SellItem_Implementation(FInventoryItemHandle ItemHandle)
{
	UInventoryItem* InventoryItem = GetInventoryItemByHandle(ItemHandle);
	if (!InventoryItem || !InventoryItem->IsValid())
		return;

	float SellPrice = InventoryItem->GetShopItem()->GetSellPrice();
	OwnerAbilitySystemComponent->ApplyModToAttribute(UMAPlayerAttributeSet::GetGoldAttribute(), EGameplayModOp::Additive, SellPrice * InventoryItem->GetStackCount());
	RemoveItem(InventoryItem);
}
bool UInventoryComponent::Server_SellItem_Validate(FInventoryItemHandle ItemHandle)
{
	return true;
}

void UInventoryComponent::GrantItem(const UPA_ShopItem* NewItem)
{
	if (!GetOwner()->HasAuthority())
		return;

	if (UInventoryItem* StackItem = GetAvaliableStackForItem(NewItem))
	{
		StackItem->AddStackCount();
		OnItemStackCountChanged.Broadcast(StackItem->GetHandle(), StackItem->GetStackCount());
		Client_ItemStackCountChanged(StackItem->GetHandle(), StackItem->GetStackCount());
	}
	else
	{
		UInventoryItem* InventoryItem = NewObject<UInventoryItem>();
		FInventoryItemHandle NewHandle = FInventoryItemHandle::CreateHandle();
		InventoryItem->InitItem(NewHandle, NewItem, OwnerAbilitySystemComponent);
		InventoryMap.Add(NewHandle, InventoryItem);
		OnItemAdded.Broadcast(InventoryItem);
		UE_LOG(LogTemp, Warning, TEXT("Server Adding Shop Item: %s, with Id: %d"), *(InventoryItem->GetShopItem()->GetItemName().ToString()), NewHandle.GetHandleId());
		Client_ItemAdded(NewHandle, NewItem);;
		CheckItemCombination(InventoryItem);
	}
}

void UInventoryComponent::ConsumeItem(UInventoryItem* Item)
{
	if (!GetOwner()->HasAuthority())
		return;

	if (!Item)
		return;

	Item->ApplyConsumeEffect();
	if (!Item->ReduceStackCount())
	{
		RemoveItem(Item);
	}
	else
	{
		OnItemStackCountChanged.Broadcast(Item->GetHandle(), Item->GetStackCount());
		Client_ItemStackCountChanged(Item->GetHandle(), Item->GetStackCount());
	}
}

void UInventoryComponent::RemoveItem(UInventoryItem* Item)
{
	if (!GetOwner()->HasAuthority())
		return;

	Item->RemoveGASModifications();
	OnItemRemoved.Broadcast(Item->GetHandle());
	InventoryMap.Remove(Item->GetHandle());
	Client_ItemRemoved(Item->GetHandle());
}

void UInventoryComponent::CheckItemCombination(const UInventoryItem* NewItem)
{
	if (!GetOwner()->HasAuthority())
		return;

	const FItemCollection* CombinationItems = UMAAssetManager::Get().GetCombinationForItem(NewItem->GetShopItem());
	if (!CombinationItems)
		return;

	for (const UPA_ShopItem* CombinationItem : CombinationItems->GetItems())
	{
		TArray<UInventoryItem*> Ingredients;
		if (!FoundIngredientForItem(CombinationItem, Ingredients))
			continue;

		for (UInventoryItem* Ingredient : Ingredients)
		{
			RemoveItem(Ingredient);
		}

		GrantItem(CombinationItem);
		return;
	}
}

void UInventoryComponent::Client_ItemRemoved_Implementation(FInventoryItemHandle ItemHandle)
{
	if (GetOwner()->HasAuthority())
		return;

	UInventoryItem* InventoryItem = GetInventoryItemByHandle(ItemHandle);
	if (!InventoryItem)
		return;

	OnItemRemoved.Broadcast(ItemHandle);
	InventoryMap.Remove(ItemHandle);
}

void UInventoryComponent::Client_ItemStackCountChanged_Implementation(FInventoryItemHandle Handle, int NewCount)
{
	if (GetOwner()->HasAuthority())
		return;

	UInventoryItem* FoundItem = GetInventoryItemByHandle(Handle);
	if (FoundItem)
	{
		FoundItem->SetStackCount(NewCount);
		OnItemStackCountChanged.Broadcast(Handle, NewCount);
	}
}

void UInventoryComponent::Client_ItemAdded_Implementation(FInventoryItemHandle AssignedHandle, const UPA_ShopItem* Item)
{
	if (GetOwner()->HasAuthority())
		return;

	UInventoryItem* InventoryItem = NewObject<UInventoryItem>();
	InventoryItem->InitItem(AssignedHandle, Item, OwnerAbilitySystemComponent);
	InventoryMap.Add(AssignedHandle, InventoryItem);
	OnItemAdded.Broadcast(InventoryItem);
	UE_LOG(LogTemp, Warning, TEXT("Client Adding Shop Item: %s, with Id: %d"), *(InventoryItem->GetShopItem()->GetItemName().ToString()), AssignedHandle.GetHandleId());
}

void UInventoryComponent::Server_Purchase_Implementation(const UPA_ShopItem* ItemToPurchase)
{
	if (!ItemToPurchase) return;

	// 1. 돈이 충분한지 확인
	if (GetGold() < ItemToPurchase->GetPrice()) return;

	// [수정] 2. 스킬인지 아이템인지 확인하여 분기 처리
	if (ItemToPurchase->GetIsSkill())
	{
		// --- [A] 스킬인 경우: 스킬북에 추가 ---
        
		// 캐릭터에서 SkillBookComponent 가져오기
		AMAPlayerCharacter* OwnerCharacter = Cast<AMAPlayerCharacter>(GetOwner());
		if (OwnerCharacter)
		{
			USkillBookComponent* SkillBook = OwnerCharacter->GetSkillBookComponent();
			if (SkillBook)
			{
				if (SkillBook->HasSkill(ItemToPurchase->GetGrantedAbility()))
				{
					UE_LOG(LogTemp, Warning, TEXT("Already learned this skill!"));
					return; 
				}
				
				OwnerAbilitySystemComponent->ApplyModToAttribute(UMAPlayerAttributeSet::GetGoldAttribute(), EGameplayModOp::Additive, -ItemToPurchase->GetPrice());
				SkillBook->UnlockSkill(ItemToPurchase->GetGrantedAbility());
                
				UE_LOG(LogTemp, Warning, TEXT("Purchased Skill: %s"), *ItemToPurchase->GetName());
			}
		}
	}
	else
	{
		if (IsFullFor(ItemToPurchase)) return;
		OwnerAbilitySystemComponent->ApplyModToAttribute(UMAPlayerAttributeSet::GetGoldAttribute(), EGameplayModOp::Additive, -ItemToPurchase->GetPrice());
		GrantItem(ItemToPurchase);
	}
}

bool UInventoryComponent::Server_Purchase_Validate(const UPA_ShopItem* ItemToPurchase)
{
	return true;
}
