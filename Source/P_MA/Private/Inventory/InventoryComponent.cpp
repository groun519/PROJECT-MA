// Fill out your copyright notice in the Description page of Project Settings.

#include "Inventory/InventoryComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Inventory/SkillBookComponent.h" 
#include "Player/MAPlayerCharacter.h"
#include "Framework/MAAssetManager.h"
#include "GAS/MAPlayerAttributeSet.h"
#include "GAS/Ability/MAGameplayAbility_Skill.h"
#include "GAS/Modules/MASkillModuleData.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (OwnerAbilitySystemComponent)
		OwnerAbilitySystemComponent->AbilityCommittedCallbacks.AddUObject(this, &UInventoryComponent::AbilityCommitted);
}

void UInventoryComponent::TryPurchaseItem(FName ItemRowName, UDataTable* SourceTable)
{
	if (!OwnerAbilitySystemComponent) return;
	Server_PurchaseItem(ItemRowName, SourceTable);
}

void UInventoryComponent::TryPurchaseSkill(FName SkillRowName, UDataTable* SourceTable)
{
	Server_PurchaseSkill(SkillRowName, SourceTable);
}

void UInventoryComponent::TryActivateItem(const FInventoryItemHandle& ItemHandle)
{
	UInventoryItem* InventoryItem = GetInventoryItemByHandle(ItemHandle);
	if (!InventoryItem) return;
	Server_ActivateItem(ItemHandle);
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
		if (bFound) return Gold;
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
	if (FoundItem) return *FoundItem;
	return nullptr;
}

bool UInventoryComponent::IsFullFor(FName ItemRowName, UDataTable* SourceTable) const
{
	if (IsAllSlotOccupied())
	{
		return GetAvaliableStackFor(ItemRowName, SourceTable) == nullptr;
	}
	return false;
}

bool UInventoryComponent::IsAllSlotOccupied() const
{
	return InventoryMap.Num() >= GetCapacity();
}

UInventoryItem* UInventoryComponent::GetAvaliableStackFor(FName ItemRowName, UDataTable* SourceTable) const
{
	if (!SourceTable) return nullptr;
	const FBaseItemData* Data = SourceTable->FindRow<FBaseItemData>(ItemRowName, TEXT("CheckStack"));
	
	if (!Data || Data->ItemType != EMAItemType::Consumable) return nullptr;

	for (const TPair<FInventoryItemHandle, UInventoryItem*>& ItemPair : InventoryMap)
	{
		if (ItemPair.Value && ItemPair.Value->IsSameItem(ItemRowName, SourceTable) && !ItemPair.Value->IsStackFull())
		{
			return ItemPair.Value;
		}
	}
	return nullptr;
}

/*
bool UInventoryComponent::FoundIngredientForItem(...) { ... }
UInventoryItem* UInventoryComponent::TryGetItemForShopItem(...) { ... }
*/

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

void UInventoryComponent::AbilityCommitted(UGameplayAbility* CommittedAbility)
{
	if (!CommittedAbility) return;

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
		if (!ItemPair.Value) continue;
		if (ItemPair.Value->IsGrantintAbility(CommittedAbility->GetClass()))
		{
			OnItemAbilityCommitted.Broadcast(ItemPair.Key, CooldownDuration, CooldownTimeRemaining);
		}
	}
}

void UInventoryComponent::Server_ActivateItem_Implementation(FInventoryItemHandle ItemHandle)
{
	UInventoryItem* InventoryItem = GetInventoryItemByHandle(ItemHandle);
	if (!InventoryItem) return;

	InventoryItem->TryActivateGrantedAbility();
	
	if (const FConsumableItemData* Data = InventoryItem->GetConsumableData())
	{
		ConsumeItem(InventoryItem);
	}
}
bool UInventoryComponent::Server_ActivateItem_Validate(FInventoryItemHandle ItemHandle) { return true; }

void UInventoryComponent::Server_SellItem_Implementation(FInventoryItemHandle ItemHandle)
{
	UInventoryItem* InventoryItem = GetInventoryItemByHandle(ItemHandle);
	if (!InventoryItem || !InventoryItem->IsValid()) return;
	
	if (const FBaseItemData* Data = InventoryItem->GetBaseData())
	{
		float SellPrice = Data->Price / 2.f;
		OwnerAbilitySystemComponent->ApplyModToAttribute(UMAPlayerAttributeSet::GetGoldAttribute(), EGameplayModOp::Additive, SellPrice * InventoryItem->GetStackCount());
	}
	RemoveItem(InventoryItem);
}
bool UInventoryComponent::Server_SellItem_Validate(FInventoryItemHandle ItemHandle) { return true; }

void UInventoryComponent::GrantItem(FName ItemRowName, UDataTable* SourceTable)
{
	if (!GetOwner()->HasAuthority()) return;
	if (!SourceTable) return;
	
	if (UInventoryItem* StackItem = GetAvaliableStackFor(ItemRowName, SourceTable))
	{
		StackItem->AddStackCount();
		OnItemStackCountChanged.Broadcast(StackItem->GetHandle(), StackItem->GetStackCount());
		Client_ItemStackCountChanged(StackItem->GetHandle(), StackItem->GetStackCount());
	}
	else
	{
		UInventoryItem* InventoryItem = NewObject<UInventoryItem>();
		FInventoryItemHandle NewHandle = FInventoryItemHandle::CreateHandle();
		
		InventoryItem->InitItem(NewHandle, ItemRowName, SourceTable, OwnerAbilitySystemComponent);
		
		InventoryMap.Add(NewHandle, InventoryItem);
		OnItemAdded.Broadcast(InventoryItem);
		
		Client_ItemAdded(NewHandle, ItemRowName, SourceTable);
	}
}

void UInventoryComponent::ConsumeItem(UInventoryItem* Item)
{
	if (!GetOwner()->HasAuthority() || !Item) return;

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
	if (!GetOwner()->HasAuthority()) return;

	Item->RemoveGASModifications();
	OnItemRemoved.Broadcast(Item->GetHandle());
	InventoryMap.Remove(Item->GetHandle());
	Client_ItemRemoved(Item->GetHandle());
}

void UInventoryComponent::Client_ItemRemoved_Implementation(FInventoryItemHandle ItemHandle)
{
	if (GetOwner()->HasAuthority()) return;
	
	OnItemRemoved.Broadcast(ItemHandle); 
	InventoryMap.Remove(ItemHandle);
}

void UInventoryComponent::Client_ItemStackCountChanged_Implementation(FInventoryItemHandle Handle, int NewCount)
{
	if (GetOwner()->HasAuthority()) return;

	UInventoryItem* FoundItem = GetInventoryItemByHandle(Handle);
	if (FoundItem)
	{
		FoundItem->SetStackCount(NewCount);
		OnItemStackCountChanged.Broadcast(Handle, NewCount);
	}
}

void UInventoryComponent::Client_ItemAdded_Implementation(FInventoryItemHandle AssignedHandle, FName ItemRowName, UDataTable* SourceTable)
{
	if (GetOwner()->HasAuthority()) return;

	UInventoryItem* InventoryItem = NewObject<UInventoryItem>();
	InventoryItem->InitItem(AssignedHandle, ItemRowName, SourceTable, OwnerAbilitySystemComponent);
	
	InventoryMap.Add(AssignedHandle, InventoryItem);
	OnItemAdded.Broadcast(InventoryItem);
}

void UInventoryComponent::Server_PurchaseItem_Implementation(FName ItemRowName, UDataTable* SourceTable)
{
	if (!SourceTable) return;
	const FBaseItemData* Data = SourceTable->FindRow<FBaseItemData>(ItemRowName, TEXT("PurchaseItem"));
	if (!Data) return;
	
	if (GetGold() < Data->Price) return;
	
	if (IsFullFor(ItemRowName, SourceTable)) return;
	
	OwnerAbilitySystemComponent->ApplyModToAttribute(UMAPlayerAttributeSet::GetGoldAttribute(), EGameplayModOp::Additive, -Data->Price);
	GrantItem(ItemRowName, SourceTable);
}
bool UInventoryComponent::Server_PurchaseItem_Validate(FName ItemRowName, UDataTable* SourceTable) { return true; }

void UInventoryComponent::Server_PurchaseSkill_Implementation(FName SkillRowName, UDataTable* SourceTable)
{
	if (!SourceTable) return;
	const FSkillData* SkillData = SourceTable->FindRow<FSkillData>(SkillRowName, TEXT("PurchaseSkill"));
	if (!SkillData || !SkillData->GrantedAbility) return;
	
	if (GetGold() < SkillData->Price) return;

	AMAPlayerCharacter* OwnerCharacter = Cast<AMAPlayerCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		if (USkillBookComponent* SkillBook = OwnerCharacter->GetSkillBookComponent())
		{
			if (SkillBook->HasSkill(SkillData->GrantedAbility)) return;
			
			OwnerAbilitySystemComponent->ApplyModToAttribute(UMAPlayerAttributeSet::GetGoldAttribute(), EGameplayModOp::Additive, -SkillData->Price);
			SkillBook->UnlockSkill(SkillData->GrantedAbility);
		}
	}
}
bool UInventoryComponent::Server_PurchaseSkill_Validate(FName SkillRowName, UDataTable* SourceTable) { return true; }