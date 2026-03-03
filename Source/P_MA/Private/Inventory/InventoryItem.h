// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilitySpecHandle.h"
#include "GAS/Modules/MASkillModuleData.h"
#include "Inventory/MAItemTypes.h" 
#include "InventoryItem.generated.h"

class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAbilityCanCastUpdatedDelegate, bool /*bCanCast*/)

USTRUCT()
struct FInventoryItemHandle
{
	GENERATED_BODY()
public:
	FInventoryItemHandle();
	static FInventoryItemHandle InvalidHandle();
	static FInventoryItemHandle CreateHandle();

	bool IsValid() const;
	uint32 GetHandleId() const { return HandleId; }
private:
	explicit FInventoryItemHandle(uint32 Id);
	UPROPERTY()
	uint32 HandleId;
	static uint32 GenerateNextId();
	static uint32 GetInvalidId();
};

FORCEINLINE bool operator==(const FInventoryItemHandle& Lhs, const FInventoryItemHandle& Rhs)
{
	return Lhs.GetHandleId() == Rhs.GetHandleId();
}

FORCEINLINE uint32 GetTypeHash(const FInventoryItemHandle& Key)
{
	return Key.GetHandleId();
}

bool operator==(const FInventoryItemHandle& Lhs, const FInventoryItemHandle& Rhs);
uint32 GetTypeHash(const FInventoryItemHandle& Key);

/**
 * 
 * 
 */
UCLASS()
class UInventoryItem : public UObject
{
	GENERATED_BODY()

public:
	UInventoryItem();

	UPROPERTY()
	FName ItemRowName;

	FOnAbilityCanCastUpdatedDelegate OnAbilityCanCastUpdated;

	bool IsSameItem(FName OtherRowName, UDataTable* OtherTable) const;
	
	void InitItem(const FInventoryItemHandle& NewHandle, FName NewRowName, UDataTable* InSourceTable, UAbilitySystemComponent* AbilitySystemComponent);
	
	const FBaseItemData* GetBaseData() const;           
	const FConsumableItemData* GetConsumableData() const; 
	const FEquipmentItemData* GetEquipmentData() const;  
	const FSkillData* GetSkillData() const;       
	
	UTexture2D* GetIcon() const;
	TSubclassOf<UMAGameplayAbility_Skill> GetGrantedAbility() const;
	bool IsStackable() const;
	int32 GetMaxStackCount() const;

	bool AddStackCount();
	bool ReduceStackCount();
	bool SetStackCount(int NewStackCount);
	bool IsStackFull() const;
	
	
	bool IsGrantintAbility(TSubclassOf<class UGameplayAbility> AbilityClass) const;
	bool IsGrantingAnyAbility() const;
	
	bool IsValid() const;
	FInventoryItemHandle GetHandle() const { return Handle; }

	bool TryActivateGrantedAbility();
	void ApplyConsumeEffect();
	void RemoveGASModifications();
	
	FORCEINLINE int GetStackCount() const { return StackCount; }
	void SetSlot(int NewSlot);
	int GetItemSlot() const { return Slot; }

	float GetAbilityCooldownTimeRemaining() const;
	float GetAbilityCooldownDuration() const;
	bool CanCastAbility() const;
	
	FGameplayAbilitySpecHandle GetGrantedAbilitySpecHandle() const { return GrantedAbiltiySpecHandle; }
	void SetGrantedAbilitySpecHandle(FGameplayAbilitySpecHandle SpecHandle) { GrantedAbiltiySpecHandle = SpecHandle; }
	
private:
	void ApplyGASModifications();

	UAbilitySystemComponent* OwnerAbilitySystemComponent;
	
	

	UPROPERTY()
	TObjectPtr<UDataTable> SourceDataTable;

	UPROPERTY()
	EMAItemType CachedType = EMAItemType::None;

	FInventoryItemHandle Handle;
	int StackCount;
	int Slot;

	TArray<FActiveGameplayEffectHandle> AppliedEquipedEffectHandles;
	FGameplayAbilitySpecHandle GrantedAbiltiySpecHandle;
};