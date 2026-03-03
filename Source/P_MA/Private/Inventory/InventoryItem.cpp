// Fill out your copyright notice in the Description page of Project Settings.

#include "Inventory/InventoryItem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/Ability/MAGameplayAbility_Skill.h"
#include "GAS/Modules/MASkillModuleData.h"

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

const FSkillData* UInventoryItem::GetSkillData() const
{
    if (CachedType == EMAItemType::Skill && SourceDataTable)
    {
        return SourceDataTable->FindRow<FSkillData>(ItemRowName, TEXT("InventoryItem_Skill"));
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

TSubclassOf<UMAGameplayAbility_Skill> UInventoryItem::GetGrantedAbility() const
{
    if (const FSkillData* SkillData = GetSkillData())
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
    // 데이터가 없거나 이펙트 배열이 비어있으면 리턴
    if (!Data || Data->ConsumeEffects.IsEmpty()) return;

    if (!OwnerAbilitySystemComponent) return;

    FGameplayEffectContextHandle EffectContext = OwnerAbilitySystemComponent->MakeEffectContext();
    EffectContext.AddSourceObject(this);

    // [변경] 배열을 순회하며 모든 이펙트 적용
    for (const TSubclassOf<UGameplayEffect>& EffectClass : Data->ConsumeEffects)
    {
        if (EffectClass)
        {
            OwnerAbilitySystemComponent->BP_ApplyGameplayEffectToSelf(EffectClass, 1, EffectContext);
        }
    }
}
void UInventoryItem::RemoveGASModifications()
{
    if (!OwnerAbilitySystemComponent) return;

    // 권한 확인 (서버에서만 이펙트 제거)
    if (OwnerAbilitySystemComponent->GetOwner()->HasAuthority())
    {
        // [변경] 저장된 모든 핸들을 순회하며 이펙트 제거
        for (const FActiveGameplayEffectHandle& HandleToRemove : AppliedEquipedEffectHandles)
        {
            if (HandleToRemove.IsValid())
            {
                OwnerAbilitySystemComponent->RemoveActiveGameplayEffect(HandleToRemove);
            }
        }
        
        // 핸들 배열 비우기
        AppliedEquipedEffectHandles.Empty();
    
        // (스킬 제거 로직 기존 유지)
        if (GrantedAbiltiySpecHandle.IsValid())
        {
            OwnerAbilitySystemComponent->SetRemoveAbilityOnEnd(GrantedAbiltiySpecHandle);
            GrantedAbiltiySpecHandle = FGameplayAbilitySpecHandle();
        }
    }
}

void UInventoryItem::ApplyGASModifications()
{
    // 1. 주인(ASC)이 없으면 중단
    if (!OwnerAbilitySystemComponent) return;

    // 2. 장비 아이템인지 확인 (데이터 테이블 접근)
    if (const FEquipmentItemData* EquipData = GetEquipmentData())
    {
        // [변경] 이펙트 배열이 비어있지 않은지 확인
        if (!EquipData->EquipEffects.IsEmpty())
        {
            // 4. 이펙트 적용 컨텍스트 생성
            FGameplayEffectContextHandle EffectContext = OwnerAbilitySystemComponent->MakeEffectContext();
            EffectContext.AddSourceObject(this);

            // [변경] 배열을 순회하며 이펙트 적용 및 핸들 저장
            for (const TSubclassOf<UGameplayEffect>& EffectClass : EquipData->EquipEffects)
            {
                if (EffectClass)
                {
                    FActiveGameplayEffectHandle ActiveHandle = OwnerAbilitySystemComponent->BP_ApplyGameplayEffectToSelf(
                        EffectClass, 
                        1.0f, 
                        EffectContext
                    );
                    
                    // 핸들 배열에 추가
                    AppliedEquipedEffectHandles.Add(ActiveHandle);
                }
            }
        }
    }

    // (스킬북 로직은 기존 유지)
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