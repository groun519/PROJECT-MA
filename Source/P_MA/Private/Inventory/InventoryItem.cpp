// Fill out your copyright notice in the Description page of Project Settings.

#include "Inventory/InventoryItem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "GAS/MAAbilitySystemStatics.h" // 님의 Statics 클래스
#include "GAS/MAAttributeSet.h" // 님의 AttributeSet 클래스
#include "Inventory/PA_ShopItem.h"

// --- FInventoryItemHandle 함수들 (변경 없음) ---

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

bool operator==(const FInventoryItemHandle& Lhs, const FInventoryItemHandle& Rhs)
{
    return Lhs.GetHandleId() == Rhs.GetHandleId();
}

uint32 GetTypeHash(const FInventoryItemHandle& Key)
{
    return Key.GetHandleId();
}

// --- UInventoryItem 함수들 (수정됨) ---

bool UInventoryItem::AddStackCount()
{
    if (IsStackFull())
    {
       return false;
    }

    ++StackCount;
    return true;
}

bool UInventoryItem::ReduceStackCount()
{
    --StackCount;
    if (StackCount <= 0)
    {
       return false;
    }

    return true;
}

bool UInventoryItem::SetStackCount(int NewStackCount)
{
    if (NewStackCount > 0 && NewStackCount <= GetShopItem()->GetMaxStackCount())
    {
       StackCount = NewStackCount;
       return true;
    }

    return false;
}

bool UInventoryItem::IsStackFull() const
{
    return StackCount >= GetShopItem()->GetMaxStackCount();
}

bool UInventoryItem::IsForItem(const UPA_ShopItem* Item) const
{
    if (!Item)
       return false;

    return GetShopItem() == Item;
}

bool UInventoryItem::IsGrantintAbility(TSubclassOf<class UGameplayAbility> AbilityClass) const
{
    if (!ShopItem)
       return false;

    TSubclassOf<UGameplayAbility> GrantedAbility = ShopItem->GetGrantedAbility();
    return GrantedAbility == AbilityClass;
}

bool UInventoryItem::IsGrantingAnyAbility() const
{
    if (!ShopItem)
       return false;

    return ShopItem->GetGrantedAbility() != nullptr;
}


UInventoryItem::UInventoryItem()
    :StackCount{1}
{
}

bool UInventoryItem::IsValid() const
{
    return ShopItem != nullptr;
}


void UInventoryItem::InitItem(const FInventoryItemHandle& NewHandle, const UPA_ShopItem* NewShopItem, UAbilitySystemComponent* AbilitySystemComponent)
{
    Handle = NewHandle;
    ShopItem = NewShopItem;

    OwnerAbilitySystemComponent = AbilitySystemComponent;
    
    // '마나' 델리게이트 구독 코드가 제거된 것이 올바릅니다. (님이 하신 대로)
    
    ApplyGASModifications();
}

bool UInventoryItem::TryActivateGrantedAbility()
{
    if (!GrantedAbiltiySpecHandle.IsValid())
       return false;

    if (OwnerAbilitySystemComponent && OwnerAbilitySystemComponent->TryActivateAbility(GrantedAbiltiySpecHandle))
       return true;

    return false;
}

void UInventoryItem::ApplyConsumeEffect()
{
    if (!ShopItem)
       return;

    TSubclassOf<UGameplayEffect> ConsumeEffect = ShopItem->GetConsumeEffect();
    if (!ConsumeEffect)
       return;

    OwnerAbilitySystemComponent->BP_ApplyGameplayEffectToSelf(ConsumeEffect, 1, OwnerAbilitySystemComponent->MakeEffectContext());
}

// [!!!] 여기가 수정된 함수입니다
void UInventoryItem::RemoveGASModifications()
{
    if (!OwnerAbilitySystemComponent)
       return;

    // '마나' 델리게이트 구독 취소 코드는 제거된 것이 올바릅니다.

    // [핵심 수정] GAS 효과/능력 제거는 '서버'에서만 실행되어야 합니다.
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
    if (!GetShopItem() || !OwnerAbilitySystemComponent)
       return;

    if (!OwnerAbilitySystemComponent->GetOwner() || !OwnerAbilitySystemComponent->GetOwner()->HasAuthority())
       return;

    TSubclassOf<UGameplayEffect> EquipEffect = GetShopItem()->GetEquippedEffect();
    if (EquipEffect)
    {
       AppliedEquipedEffectHandle = OwnerAbilitySystemComponent->BP_ApplyGameplayEffectToSelf(EquipEffect, 1, OwnerAbilitySystemComponent->MakeEffectContext());
    }

    TSubclassOf<UGameplayAbility> GrantedAbility = GetShopItem()->GetGrantedAbility();
    if (GrantedAbility)
    {
       GrantedAbiltiySpecHandle = OwnerAbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(GrantedAbility));
    }
}

void UInventoryItem::SetSlot(int NewSlot)
{
    Slot = NewSlot;
}

float UInventoryItem::GetAbilityCooldownTimeRemaining() const
{
    if (!IsGrantingAnyAbility())
       return 0.f;

    // 님의 Statics 클래스 이름(UMAAbilitySystemStatics)으로 수정
    return UMAAbilitySystemStatics::GetCooldownRemainingFor(GetShopItem()->GetGrantedAbilityCDO(), *OwnerAbilitySystemComponent);
}

float UInventoryItem::GetAbilityCooldownDuration() const
{
    if (!IsGrantingAnyAbility())
       return 0.f;

    // 님의 Statics 클래스 이름(UMAAbilitySystemStatics)으로 수정
    return UMAAbilitySystemStatics::GetCooldownDurationFor(GetShopItem()->GetGrantedAbilityCDO(), *OwnerAbilitySystemComponent, 1);
}

// 'GetAbilityManaCost' 함수는 '마나'가 없으므로 제거된 것이 올바릅니다.

bool UInventoryItem::CanCastAbility() const
{
    if (!IsGrantingAnyAbility() || !OwnerAbilitySystemComponent)
       return false;

    FGameplayAbilitySpec* Spec = OwnerAbilitySystemComponent->FindAbilitySpecFromHandle(GrantedAbiltiySpecHandle);
    if (Spec)
    {
        // 님의 Statics 클래스 이름(UMAAbilitySystemStatics)으로 수정
       return UMAAbilitySystemStatics::CheckAbilityCost(*Spec, *OwnerAbilitySystemComponent);
    }

    // 님의 Statics 클래스 이름(UMAAbilitySystemStatics)으로 수정
    return UMAAbilitySystemStatics::CheckAbilityCostStatic(GetShopItem()->GetGrantedAbilityCDO(), *OwnerAbilitySystemComponent);
}