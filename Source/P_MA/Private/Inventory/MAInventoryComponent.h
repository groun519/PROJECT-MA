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

	/** Module **/
	bool RequestGrantModule(UMASkillModule* Module);

	/** Item **/
	bool RequestGrantItem(FName ItemRowName, int32 Count);
	bool RequestConsumeItem(int32 EntryId, int32 Count);

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
	int32 GetSlotCount() const { return FMath::Max(0, MaxSlotCount); }
	const FMAInventoryEntry* GetEntryAt(int32 SlotIndex) const;
	UMASkillModuleInstance* GetModuleAt(int32 SlotIndex) const;

private:
	/** Module **/
	bool AddModule(UMASkillModule* Module);

	/** Item **/
	bool AddItem(FName ItemRowName, int32 Count);
	bool ConsumeItem(int32 EntryId, int32 Count);

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

	/** Replication **/
	UFUNCTION(Server, Reliable)
	void ServerGrantModule(UMASkillModule* Module);

	UFUNCTION(Server, Reliable)
	void ServerGrantItem(FName ItemRowName, int32 Count);

	UFUNCTION(Server, Reliable)
	void ServerConsumeItem(int32 EntryId, int32 Count);

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
