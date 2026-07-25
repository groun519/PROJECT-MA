#pragma once

#include "CoreMinimal.h"
#include "MAInventoryTypes.generated.h"

class UMASkillModuleInstance;

UENUM()
enum class EMAInventoryEntryKind : uint8
{
	Empty,
	Module,
	Item
};

USTRUCT(BlueprintType)
struct P_MA_API FMAItemStack
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item")
	FName ItemRowName = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item")
	int32 Count = 0;
};

USTRUCT(BlueprintType)
struct P_MA_API FMAInventoryEntry
{
	GENERATED_BODY()

	bool IsEmpty() const { return Kind == EMAInventoryEntryKind::Empty; }
	bool IsModule() const { return Kind == EMAInventoryEntryKind::Module && ModuleInstance != nullptr; }
	bool IsItem() const { return Kind == EMAInventoryEntryKind::Item && !ItemStack.ItemRowName.IsNone() && ItemStack.Count > 0; }

	void Reset()
	{
		EntryId = INDEX_NONE;
		Kind = EMAInventoryEntryKind::Empty;
		ModuleInstance = nullptr;
		ItemStack = FMAItemStack();
	}

	void SetModule(int32 InEntryId, UMASkillModuleInstance* InModuleInstance)
	{
		Reset();
		EntryId = InEntryId;
		Kind = EMAInventoryEntryKind::Module;
		ModuleInstance = InModuleInstance;
	}

	void SetItem(int32 InEntryId, FName InItemRowName, int32 InCount)
	{
		Reset();
		EntryId = InEntryId;
		Kind = EMAInventoryEntryKind::Item;
		ItemStack.ItemRowName = InItemRowName;
		ItemStack.Count = InCount;
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	int32 EntryId = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	EMAInventoryEntryKind Kind = EMAInventoryEntryKind::Empty;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TObjectPtr<UMASkillModuleInstance> ModuleInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	FMAItemStack ItemStack;
};
