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

UInventoryItem::UInventoryItem()
    : StackCount{1}
{
}

// [핵심 변경] 초기화 함수
void UInventoryItem::InitItem(const FInventoryItemHandle& NewHandle, FName NewRowName, UDataTable* InSourceTable, UAbilitySystemComponent* AbilitySystemComponent)
{
    Handle = NewHandle;
    ItemRowName = NewRowName;
    SourceDataTable = InSourceTable;
    OwnerAbilitySystemComponent = AbilitySystemComponent;

    // 타입 미리 캐싱 (매번 검색하지 않도록)
    if (const FBaseItemData* BaseData = GetBaseData())
    {
        CachedType = BaseData->ItemType;
    }

    ApplyGASModifications();
}

// --- [추가] 데이터 접근 구현 ---
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
// ---------------------------

// [어댑터] 아이콘 가져오기
UTexture2D* UInventoryItem::GetIcon() const
{
    if (const FBaseItemData* Data = GetBaseData())
    {
        return Data->Icon.LoadSynchronous();
    }
    return nullptr;
}

// [어댑터] 스택 가능 여부
bool UInventoryItem::IsStackable() const
{
    // 소비 아이템만 스택 가능 (기획에 따라 변경 가능)
    return CachedType == EMAItemType::Consumable;
}

// [어댑터] 최대 스택 수
int32 UInventoryItem::GetMaxStackCount() const
{
    if (const FConsumableItemData* Data = GetConsumableData())
    {
        return Data->MaxStackCount;
    }
    return 1; // 장비나 스킬은 1개
}

// [어댑터] 부여된 스킬 가져오기 (상속 구조 활용)
TSubclassOf<UGameplayAbility> UInventoryItem::GetGrantedAbility() const
{
    // 스킬북인 경우
    if (const FSkillItemData* SkillData = GetSkillData())
    {
        return SkillData->GrantedAbility;
    }
    // 소비 아이템이 즉시 사용 스킬을 가질 수도 있다면 여기에 추가 로직
    return nullptr;
}

// --- 기존 로직 수정 ---

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
    if (NewStackCount > 0 && NewStackCount <= GetMaxStackCount()) // 함수 교체
    {
       StackCount = NewStackCount;
       return true;
    }
    return false;
}

bool UInventoryItem::IsStackFull() const
{
    return StackCount >= GetMaxStackCount(); // 함수 교체
}

// [삭제/보류] 이 함수는 InventoryComponent에서 파라미터를 바꾼 뒤 다시 살려야 합니다.
/*
bool UInventoryItem::IsForItem(const UPA_ShopItem* Item) const
{
    // ...
}
*/

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
    // 소비 아이템 데이터 확인
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

    // 1. 장비 이펙트 적용 (장비인 경우)
    if (const FEquipmentItemData* EquipData = GetEquipmentData())
    {
        if (EquipData->EquipEffect)
        {
            AppliedEquipedEffectHandle = OwnerAbilitySystemComponent->BP_ApplyGameplayEffectToSelf(EquipData->EquipEffect, 1, OwnerAbilitySystemComponent->MakeEffectContext());
        }
    }

    // 2. 스킬 부여 (스킬북인 경우)
    if (TSubclassOf<UGameplayAbility> AbilityToGrant = GetGrantedAbility())
    {
       GrantedAbiltiySpecHandle = OwnerAbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityToGrant));
    }
}

void UInventoryItem::SetSlot(int NewSlot)
{
    Slot = NewSlot;
}

// ... (GetAbilityCooldownTimeRemaining 등은 GetGrantedAbility() 함수를 쓰므로 수정 없이 자동 적용됨) ...
float UInventoryItem::GetAbilityCooldownTimeRemaining() const
{
    if (!IsGrantingAnyAbility()) return 0.f;
    
    // CDO 가져오는 부분이 약간 까다로운데, 일단 클래스로 찾습니다.
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
    // 행 이름과 출신 데이터 테이블이 모두 같으면 같은 아이템입니다.
    return (ItemRowName == OtherRowName) && (SourceDataTable == OtherTable);
}