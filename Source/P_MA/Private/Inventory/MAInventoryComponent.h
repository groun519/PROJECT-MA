#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Inventory/MAInventoryTypes.h"
#include "MAInventoryComponent.generated.h"

class UMASkillModule;
class UMASkillModuleInstance;

DECLARE_MULTICAST_DELEGATE(FMAInventoryChangedSignature);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MA_API UMAInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMAInventoryComponent();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FMAInventoryChangedSignature OnInventoryChanged;

	/** Module Addition **/
	bool RequestAddModule(int32 ModuleId, int32 Count = 1);

	/** Item **/
	void UseItem(int32 EntryId);

	/** Stack **/
	bool SetStackCount(int32 EntryId, int32 NewCount);

	/** Entry Transfer **/
	bool RequestMoveEntry(int32 EntryId, int32 TargetSlotIndex);
	bool RequestEquipModule(
		int32 EntryId,
		FGameplayTag SkillSlotTag,
		int32 ModuleIndex);
	bool RequestMoveSkillModuleToInventory(
		FGameplayTag SkillSlotTag,
		int32 ModuleIndex,
		int32 TargetSlotIndex);

	/** Query **/
	const TArray<FMAInventoryEntry>& GetEntries() const { return Entries; }
	const FMAInventoryEntry* GetEntryAt(int32 SlotIndex) const
	{
		return Entries.IsValidIndex(SlotIndex) ? &Entries[SlotIndex] : nullptr;
	}
	const FMAInventoryEntry* FindEntry(int32 EntryId) const;

private:
	/** Module Addition **/
	bool AddModule(int32 ModuleId, int32 Count);
	bool AddModuleInstance(UMASkillModule* Module);
	bool AddStack(UMASkillModule* Module, int32 Count);

	/** Item **/
	EMAItemUseResult ExecuteUseItem(int32 EntryId);

	/** Entry Transfer **/
	bool MoveEntry(int32 EntryId, int32 TargetSlotIndex);
	bool EquipModule(
		int32 EntryId,
		FGameplayTag SkillSlotTag,
		int32 ModuleIndex);
	bool MoveSkillModuleToInventory(
		FGameplayTag SkillSlotTag,
		int32 ModuleIndex,
		int32 TargetSlotIndex);

	/** Internal **/
	bool CanMutateInventory() const;
	int32 AllocateEntryId();
	int32 FindEntrySlot(int32 EntryId) const;
	void EnsureSlotCount();
	void RefreshEntryModuleStates();
	void NotifyInventoryChanged();
	void ReportItemUseResult(int32 EntryId, EMAItemUseResult Result) const;

	/** Replication **/
	UFUNCTION(Server, Reliable)
	void ServerAddModule(int32 ModuleId, int32 Count);

	UFUNCTION(Server, Reliable)
	void ServerUseItem(int32 EntryId);

	UFUNCTION(Server, Reliable)
	void ServerMoveEntry(int32 EntryId, int32 TargetSlotIndex);

	UFUNCTION(Server, Reliable)
	void ServerEquipModule(
		int32 EntryId,
		FGameplayTag SkillSlotTag,
		int32 ModuleIndex);

	UFUNCTION(Server, Reliable)
	void ServerMoveSkillModuleToInventory(
		FGameplayTag SkillSlotTag,
		int32 ModuleIndex,
		int32 TargetSlotIndex);

	UFUNCTION()
	void OnRep_Entries();

	UPROPERTY(EditDefaultsOnly, Category="Inventory", meta=(ClampMin="0"))
	int32 MaxSlotCount = 30;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_Entries)
	TArray<FMAInventoryEntry> Entries;

	int32 NextEntryId = 1;
};
