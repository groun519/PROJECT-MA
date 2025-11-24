// Fill out your copyright notice in the Description page of Project Settings.

#include "Inventory/InventoryItem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAAttributeSet.h" // 님의 AttributeSet 클래스
#include "Inventory/PA_ShopItem.h"

FInventoryItemHandle::FInventoryItemHandle()
    : HandleId{GetInvalidId()}
{
}

FInventoryItemHandle FInventoryItemHandle::InvalidHandle()
{
    static FInventoryItemHandle InvalidHandle = FInventoryItemHandle();
    return InvalidHandle;
}

FInventoryItemHandle::FInventoryItemHandle(uint32 Id)
    : HandleId{Id}
{
}

FInventoryItemHandle FInventoryItemHandle::CreateHandle()
{
    return FInventoryItemHandle(GenerateNextId());
}

bool FInventoryItemHandle::IsValid() const
{
    return HandleId != GetInvalidId();
}

uint32 FInventoryItemHandle::GenerateNextId()
{
    static uint32 StaticId = 1;
    return StaticId++;
}

uint32 FInventoryItemHandle::GetInvalidId()
{
    return 0;
}

UInventoryItem::UInventoryItem()
    : StackCount{1}
{
}

void UInventoryItem::InitItem(const FInventoryItemHandle& NewHandle, FName NewRowName, UDataTable* InSourceTable, UAbilitySystemComponent* AbilitySystemComponent)
{
    Handle = NewHandle;
    ItemRowName = NewRowName;
    SourceDataTable = InSourceTable;
    OwnerAbilitySystemComponent = AbilitySystemComponent;
    
    if (const FBaseItemData* BaseData = GetBaseData())
    {
        CachedType = BaseData->ItemType;
    }

    ApplyGASModifications();
}

const FBaseItemData* UInventoryItem::GetBaseData() const
{
    if (SourceDataTable && !ItemRowName.IsNone())
    {
        return SourceDataTable->FindRow<FBaseItemData>(ItemRowName, TEXT("InventoryItem_Base"));
    }
    return nullptr;
}

const FConsumableItemData* UInventoryItem::GetConsumableData() const
{
    if (CachedType == EMAItemType::Consumable && SourceDataTable)
    {
        return SourceDataTable->FindRow<FConsumableItemData>(ItemRowName, TEXT("InventoryItem_Consumable"));
    }
    return nullptr;
}

const FEquipmentItemData* UInventoryItem::GetEquipmentData() const
{
    if (CachedType == EMAItemType::Equipment && SourceDataTable)
    {
        return SourceDataTable->FindRow<FEquipmentItemData>(ItemRowName, TEXT("InventoryItem_Equip"));
    }
    return nullptr;
}

const FSkillItemData* UInventoryItem::GetSkillData() const
{
    if (CachedType == EMAItemType::Skill && SourceDataTable)
    {
        return SourceDataTable->FindRow<FSkillItemData>(ItemRowName, TEXT("InventoryItem_Skill"));
    }
    return nullptr;
}

UTexture2D* UInventoryItem::GetIcon() const
{
    if (const FBaseItemData* Data = GetBaseData())
    {
        return Data->Icon.LoadSynchronous();
    }
    return nullptr;
}

bool UInventoryItem::IsStackable() const
{
    return CachedType == EMAItemType::Consumable;
}

int32 UInventoryItem::GetMaxStackCount() const
{
    if (const FConsumableItemData* Data = GetConsumableData())
    {
        return Data->MaxStackCount;
    }
    return 1;
}

TSubclassOf<UGameplayAbility> UInventoryItem::GetGrantedAbility() const
{
    if (const FSkillItemData* SkillData = GetSkillData())
    {
        return SkillData->GrantedAbility;
    }
    return nullptr;
}


bool UInventoryItem::AddStackCount()
{
    if (IsStackFull()) return false;
    ++StackCount;
    return true;
}

bool UInventoryItem::ReduceStackCount()
{
    --StackCount;
    if (StackCount <= 0) return false;
    return true;
}

bool UInventoryItem::SetStackCount(int NewStackCount)
{
    if (NewStackCount > 0 && NewStackCount <= GetMaxStackCount()) 
    {
       StackCount = NewStackCount;
       return true;
    }
    return false;
}

bool UInventoryItem::IsStackFull() const
{
    return StackCount >= GetMaxStackCount();
}

bool UInventoryItem::IsGrantintAbility(TSubclassOf<class UGameplayAbility> AbilityClass) const
{
    TSubclassOf<UGameplayAbility> GrantedAbility = GetGrantedAbility();
    return GrantedAbility && (GrantedAbility == AbilityClass);
}

bool UInventoryItem::IsGrantingAnyAbility() const
{
    return GetGrantedAbility() != nullptr;
}

bool UInventoryItem::IsValid() const
{
    return SourceDataTable != nullptr && !ItemRowName.IsNone();
}

bool UInventoryItem::TryActivateGrantedAbility()
{
    if (!GrantedAbiltiySpecHandle.IsValid()) return false;
    if (OwnerAbilitySystemComponent && OwnerAbilitySystemComponent->TryActivateAbility(GrantedAbiltiySpecHandle)) return true;
    return false;
}

void UInventoryItem::ApplyConsumeEffect()
{
    const FConsumableItemData* Data = GetConsumableData();
    if (!Data || !Data->ConsumeEffect) return;

    OwnerAbilitySystemComponent->BP_ApplyGameplayEffectToSelf(Data->ConsumeEffect, 1, OwnerAbilitySystemComponent->MakeEffectContext());
}

void UInventoryItem::RemoveGASModifications()
{
    if (!OwnerAbilitySystemComponent) return;

    if (OwnerAbilitySystemComponent->GetOwner()->HasAuthority())
    {
        if (AppliedEquipedEffectHandle.IsValid())
           OwnerAbilitySystemComponent->RemoveActiveGameplayEffect(AppliedEquipedEffectHandle);
    
        if (GrantedAbiltiySpecHandle.IsValid())
           OwnerAbilitySystemComponent->SetRemoveAbilityOnEnd(GrantedAbiltiySpecHandle);
    }
}

void UInventoryItem::ApplyGASModifications()
{
    if (!OwnerAbilitySystemComponent) return;
    if (!OwnerAbilitySystemComponent->GetOwner() || !OwnerAbilitySystemComponent->GetOwner()->HasAuthority()) return;
    
    if (const FEquipmentItemData* EquipData = GetEquipmentData())
    {
        if (EquipData->EquipEffect)
        {
            AppliedEquipedEffectHandle = OwnerAbilitySystemComponent->BP_ApplyGameplayEffectToSelf(EquipData->EquipEffect, 1, OwnerAbilitySystemComponent->MakeEffectContext());
        }
    }
    
    if (TSubclassOf<UGameplayAbility> AbilityToGrant = GetGrantedAbility())
    {
       GrantedAbiltiySpecHandle = OwnerAbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityToGrant));
    }
}

void UInventoryItem::SetSlot(int NewSlot)
{
    Slot = NewSlot;
}

float UInventoryItem::GetAbilityCooldownTimeRemaining() const
{
    if (!IsGrantingAnyAbility()) return 0.f;
    
    UGameplayAbility* CDO = GetGrantedAbility()->GetDefaultObject<UGameplayAbility>();
    return UMAAbilitySystemStatics::GetCooldownRemainingFor(CDO, *OwnerAbilitySystemComponent);
}

float UInventoryItem::GetAbilityCooldownDuration() const
{
    if (!IsGrantingAnyAbility()) return 0.f;
    UGameplayAbility* CDO = GetGrantedAbility()->GetDefaultObject<UGameplayAbility>();
    return UMAAbilitySystemStatics::GetCooldownDurationFor(CDO, *OwnerAbilitySystemComponent, 1);
}

bool UInventoryItem::CanCastAbility() const
{
    if (!IsGrantingAnyAbility() || !OwnerAbilitySystemComponent) return false;

    FGameplayAbilitySpec* Spec = OwnerAbilitySystemComponent->FindAbilitySpecFromHandle(GrantedAbiltiySpecHandle);
    if (Spec)
    {
       return UMAAbilitySystemStatics::CheckAbilityCost(*Spec, *OwnerAbilitySystemComponent);
    }
    
    UGameplayAbility* CDO = GetGrantedAbility()->GetDefaultObject<UGameplayAbility>();
    return UMAAbilitySystemStatics::CheckAbilityCostStatic(CDO, *OwnerAbilitySystemComponent);
}

bool UInventoryItem::IsSameItem(FName OtherRowName, UDataTable* OtherTable) const
{
    return (ItemRowName == OtherRowName) && (SourceDataTable == OtherTable);
}