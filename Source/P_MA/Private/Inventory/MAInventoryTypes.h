#pragma once

#include "CoreMinimal.h"
#include "MAInventoryTypes.generated.h"

class UMASkillModule;
class UMASkillModuleInstance;

UENUM(BlueprintType)
enum class EMAItemUseResult : uint8
{
	Success,
	NotUsable,
	InvalidEntry
};

UENUM()
enum class EMAInventoryEntryKind : uint8
{
	Empty,
	ModuleInstance,
	Stack
};

USTRUCT(BlueprintType)
struct P_MA_API FMAInventoryStack
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TObjectPtr<UMASkillModule> Module;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	int32 Count = 0;
};

USTRUCT(BlueprintType)
struct P_MA_API FMAInventoryEntry
{
	GENERATED_BODY()

	bool IsEmpty() const { return Kind == EMAInventoryEntryKind::Empty; }
	bool IsModuleInstance() const
	{
		return Kind == EMAInventoryEntryKind::ModuleInstance && ModuleInstance != nullptr;
	}
	bool IsStack() const { return Kind == EMAInventoryEntryKind::Stack && Stack.Module && Stack.Count > 0; }

	UMASkillModuleInstance* GetModuleInstance() const
	{
		return IsModuleInstance() ? ModuleInstance.Get() : nullptr;
	}
	const FMAInventoryStack* GetStack() const { return IsStack() ? &Stack : nullptr; }

	void Reset()
	{
		EntryId = INDEX_NONE;
		Kind = EMAInventoryEntryKind::Empty;
		ModuleInstance = nullptr;
		Stack = FMAInventoryStack();
	}

	void SetModuleInstance(int32 InEntryId, UMASkillModuleInstance* InModuleInstance)
	{
		Reset();
		EntryId = InEntryId;
		Kind = EMAInventoryEntryKind::ModuleInstance;
		ModuleInstance = InModuleInstance;
	}

	void SetStack(int32 InEntryId, UMASkillModule* InModule, int32 InCount)
	{
		Reset();
		EntryId = InEntryId;
		Kind = EMAInventoryEntryKind::Stack;
		Stack.Module = InModule;
		Stack.Count = InCount;
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	int32 EntryId = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	EMAInventoryEntryKind Kind = EMAInventoryEntryKind::Empty;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TObjectPtr<UMASkillModuleInstance> ModuleInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	FMAInventoryStack Stack;
};
