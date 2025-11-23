// Fill out your copyright notice in the Description page of Project Settings.

#include "Inventory/InventoryComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Inventory/SkillBookComponent.h" 
#include "Player/MAPlayerCharacter.h"
#include "Framework/MAAssetManager.h"
#include "GAS/MAPlayerAttributeSet.h"
// #include "Inventory/PA_ShopItem.h" // [삭제]

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

// --- [변경] 구매 요청 (클라이언트 -> 서버) ---
void UInventoryComponent::TryPurchaseItem(FName ItemRowName, UDataTable* SourceTable)
{
	if (!OwnerAbilitySystemComponent) return;
	Server_PurchaseItem(ItemRowName, SourceTable);
}

void UInventoryComponent::TryPurchaseSkill(FName SkillRowName, UDataTable* SourceTable)
{
	Server_PurchaseSkill(SkillRowName, SourceTable);
}
// --------------------------------------------

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

// [변경] 아이템 공간 확인
bool UInventoryComponent::IsFullFor(FName ItemRowName, UDataTable* SourceTable) const
{
	if (IsAllSlotOccupied())
	{
		// 스택 가능한지 확인 (이미 있는 아이템에 얹을 수 있는지)
		return GetAvaliableStackFor(ItemRowName, SourceTable) == nullptr;
	}
	return false;
}

bool UInventoryComponent::IsAllSlotOccupied() const
{
	return InventoryMap.Num() >= GetCapacity();
}

// [변경] 스택 가능한 아이템 찾기 (IsSameItem 사용)
UInventoryItem* UInventoryComponent::GetAvaliableStackFor(FName ItemRowName, UDataTable* SourceTable) const
{
	// 1. 데이터 테이블에서 아이템 정보 확인 (Stackable인지)
	if (!SourceTable) return nullptr;
	const FBaseItemData* Data = SourceTable->FindRow<FBaseItemData>(ItemRowName, TEXT("CheckStack"));
	
	// 소비 아이템이 아니면 스택 불가 (기본적으로)
	if (!Data || Data->ItemType != EMAItemType::Consumable) return nullptr;

	for (const TPair<FInventoryItemHandle, UInventoryItem*>& ItemPair : InventoryMap)
	{
		// 같은 아이템이고, 스택이 가득 차지 않았으면 반환
		if (ItemPair.Value && ItemPair.Value->IsSameItem(ItemRowName, SourceTable) && !ItemPair.Value->IsStackFull())
		{
			return ItemPair.Value;
		}
	}
	return nullptr;
}

// 조합 관련 함수는 구조 변경으로 인해 잠시 주석 처리합니다.
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

// --- 서버 로직들 ---

void UInventoryComponent::Server_ActivateItem_Implementation(FInventoryItemHandle ItemHandle)
{
	UInventoryItem* InventoryItem = GetInventoryItemByHandle(ItemHandle);
	if (!InventoryItem) return;

	InventoryItem->TryActivateGrantedAbility();
	
	// 소비 아이템인지 확인
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

	// 가격 정보 가져오기
	if (const FBaseItemData* Data = InventoryItem->GetBaseData())
	{
		float SellPrice = Data->Price / 2.f;
		OwnerAbilitySystemComponent->ApplyModToAttribute(UMAPlayerAttributeSet::GetGoldAttribute(), EGameplayModOp::Additive, SellPrice * InventoryItem->GetStackCount());
	}
	RemoveItem(InventoryItem);
}
bool UInventoryComponent::Server_SellItem_Validate(FInventoryItemHandle ItemHandle) { return true; }


// [변경] 아이템 지급 (핵심)
void UInventoryComponent::GrantItem(FName ItemRowName, UDataTable* SourceTable)
{
	if (!GetOwner()->HasAuthority()) return;
	if (!SourceTable) return;

	// 1. 스택 가능한지 확인
	if (UInventoryItem* StackItem = GetAvaliableStackFor(ItemRowName, SourceTable))
	{
		StackItem->AddStackCount();
		OnItemStackCountChanged.Broadcast(StackItem->GetHandle(), StackItem->GetStackCount());
		Client_ItemStackCountChanged(StackItem->GetHandle(), StackItem->GetStackCount());
	}
	else
	{
		// 2. 새 아이템 생성
		UInventoryItem* InventoryItem = NewObject<UInventoryItem>();
		FInventoryItemHandle NewHandle = FInventoryItemHandle::CreateHandle();
		
		// [중요] InitItem 호출 (DataTable, RowName 전달)
		InventoryItem->InitItem(NewHandle, ItemRowName, SourceTable, OwnerAbilitySystemComponent);
		
		InventoryMap.Add(NewHandle, InventoryItem);
		OnItemAdded.Broadcast(InventoryItem);
		
		Client_ItemAdded(NewHandle, ItemRowName, SourceTable);
		
		// CheckItemCombination(InventoryItem); // 조합 로직 잠시 꺼둠
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

// --- 클라이언트 동기화 로직 ---

void UInventoryComponent::Client_ItemRemoved_Implementation(FInventoryItemHandle ItemHandle)
{
	if (GetOwner()->HasAuthority()) return;
	
	// UI 등에서 참조 해제
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

// [변경] 클라이언트 아이템 추가
void UInventoryComponent::Client_ItemAdded_Implementation(FInventoryItemHandle AssignedHandle, FName ItemRowName, UDataTable* SourceTable)
{
	if (GetOwner()->HasAuthority()) return;

	UInventoryItem* InventoryItem = NewObject<UInventoryItem>();
	// 클라이언트에서도 데이터 테이블을 참조하여 초기화
	InventoryItem->InitItem(AssignedHandle, ItemRowName, SourceTable, OwnerAbilitySystemComponent);
	
	InventoryMap.Add(AssignedHandle, InventoryItem);
	OnItemAdded.Broadcast(InventoryItem);
}


// --- 구매 관련 서버 로직 (RPC) ---

void UInventoryComponent::Server_PurchaseItem_Implementation(FName ItemRowName, UDataTable* SourceTable)
{
	if (!SourceTable) return;
	const FBaseItemData* Data = SourceTable->FindRow<FBaseItemData>(ItemRowName, TEXT("PurchaseItem"));
	if (!Data) return;

	// 1. 돈 확인
	if (GetGold() < Data->Price) return;

	// 2. 공간 확인 (장비, 소비 구분)
	if (IsFullFor(ItemRowName, SourceTable)) return;

	// 3. 구매 처리
	OwnerAbilitySystemComponent->ApplyModToAttribute(UMAPlayerAttributeSet::GetGoldAttribute(), EGameplayModOp::Additive, -Data->Price);
	GrantItem(ItemRowName, SourceTable);
}
bool UInventoryComponent::Server_PurchaseItem_Validate(FName ItemRowName, UDataTable* SourceTable) { return true; }

void UInventoryComponent::Server_PurchaseSkill_Implementation(FName SkillRowName, UDataTable* SourceTable)
{
	if (!SourceTable) return;
	const FSkillItemData* SkillData = SourceTable->FindRow<FSkillItemData>(SkillRowName, TEXT("PurchaseSkill"));
	if (!SkillData || !SkillData->GrantedAbility) return;

	// 1. 돈 확인
	if (GetGold() < SkillData->Price) return;

	AMAPlayerCharacter* OwnerCharacter = Cast<AMAPlayerCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		if (USkillBookComponent* SkillBook = OwnerCharacter->GetSkillBookComponent())
		{
			// 2. 중복 확인
			if (SkillBook->HasSkill(SkillData->GrantedAbility)) return;

			// 3. 구매 및 해금
			OwnerAbilitySystemComponent->ApplyModToAttribute(UMAPlayerAttributeSet::GetGoldAttribute(), EGameplayModOp::Additive, -SkillData->Price);
			SkillBook->UnlockSkill(SkillData->GrantedAbility);
		}
	}
}
bool UInventoryComponent::Server_PurchaseSkill_Validate(FName SkillRowName, UDataTable* SourceTable) { return true; }