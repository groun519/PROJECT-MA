#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MASkillModuleInventoryComponent.generated.h"

class UMASkillDefinition;
class UMASkillModuleInstance;
class UActorChannel;
class FOutBunch;
struct FReplicationFlags;

DECLARE_MULTICAST_DELEGATE(FMASkillModuleInventoryChangedSignature);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MA_API UMASkillModuleInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMASkillModuleInventoryComponent();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	FMASkillModuleInventoryChangedSignature OnInventoryChanged;

	bool RequestGrantModule(UMASkillDefinition* Definition);
	bool RequestMoveModuleSlot(
		const TArray<TObjectPtr<UMASkillModuleInstance>>* SourceSlots,
		int32 SourceIndex,
		UActorComponent* TargetOwner,
		const TArray<TObjectPtr<UMASkillModuleInstance>>* TargetSlots,
		int32 TargetIndex);
	bool RequestMoveSkillSlotToInventorySlot(EMAAbilityInputID InputID, int32 ModuleIndex, int32 TargetSlotIndex);
	const TArray<TObjectPtr<UMASkillModuleInstance>>* GetModuleSlotsForUI();

private:
	bool AddModule(UMASkillDefinition* Definition);
	bool CanMutateInventory() const;
	bool EquipInventorySlotToSkillSlot(int32 SourceSlotIndex, EMAAbilityInputID InputID, int32 ModuleIndex);
	bool SwapInventorySlots(int32 SourceSlotIndex, int32 TargetSlotIndex);
	bool MoveSkillSlotToInventorySlot(EMAAbilityInputID InputID, int32 ModuleIndex, int32 TargetSlotIndex);
	bool IsValidSlotIndex(int32 SlotIndex) const;
	void EnsureSlotCount();
	UFUNCTION(Server, Reliable)
	void ServerGrantModule(UMASkillDefinition* Definition);

	UFUNCTION(Server, Reliable)
	void ServerEquipInventorySlotToSkillSlot(int32 SourceSlotIndex, EMAAbilityInputID InputID, int32 ModuleIndex);

	UFUNCTION(Server, Reliable)
	void ServerSwapInventorySlots(int32 SourceSlotIndex, int32 TargetSlotIndex);

	UFUNCTION(Server, Reliable)
	void ServerMoveSkillSlotToInventorySlot(EMAAbilityInputID InputID, int32 ModuleIndex, int32 TargetSlotIndex);

	UFUNCTION()
	void OnRep_Entries();

	UPROPERTY(EditDefaultsOnly, Category="Skill Module Inventory", meta=(ClampMin="0"))
	int32 MaxSlotCount = 30;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_Entries)
	TArray<TObjectPtr<UMASkillModuleInstance>> Entries;
};
