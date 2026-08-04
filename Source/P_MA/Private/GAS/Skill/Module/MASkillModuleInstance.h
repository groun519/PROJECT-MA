#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Module/MASkillComposedModule.h"
#include "GAS/Skill/Module/MASkillModuleAddonRuntimeData.h"
#include "GAS/Skill/MASkillSystemTypes.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "UObject/Object.h"
#include "MASkillModuleInstance.generated.h"

class UMASkillManagerComponent;
class UMASkillModule;
class UMASkillModuleInstance;
class UMAInventoryComponent;
class UMASkillRuntimeRegistry;

DECLARE_MULTICAST_DELEGATE(FMASkillModuleStateChangedSignature);
DECLARE_MULTICAST_DELEGATE_OneParam(
	FMASkillSubModulesChangedSignature,
	UMASkillModuleInstance*);

UCLASS()
class P_MA_API UMASkillModuleInstance : public UObject
{
	GENERATED_BODY()

public:
	UMASkillModuleInstance();

	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UMASkillModule* GetRootModule() const { return ModuleGroup.RootModule; }
	const FMASkillModuleGroup& GetModuleGroup() const { return ModuleGroup; }
	const UMASkillModule* ResolveComposedModule();
	bool SetRootModule(UMASkillModule* InRootModule);
	bool IsValid() const { return ModuleGroup.RootModule != nullptr; }
	bool SetSubModuleAt(int32 SubModuleIndex, UMASkillModule* SubModule);
	bool IsInSkillSlot() const { return bIsInSkillSlot; }
	bool IsAssemblyActive() const { return IsValid() && IsInSkillSlot() && IsActive(); }
	bool IsActive() const { return bIsActive; }
	void SetActive(
		bool bInActive,
		const FGameplayTag& InInactiveReasonTag = FGameplayTag());
	const FGameplayTag& GetInactiveReasonTag() const { return InactiveReasonTag; }

	/** Addon Runtime Data **/
	const FMASkillModuleAddonRuntimeData& GetAddonRuntimeData() const { return AddonRuntimeData; }

	template<typename DataType, typename MutatorType>
	bool ModifyAddonRuntimeData(MutatorType&& Mutator)
	{
		if (!CanModifyAddonRuntimeData()) return false;
		if (!AddonRuntimeData.Modify<DataType>(Forward<MutatorType>(Mutator))) return false;

		NotifyAddonRuntimeDataChanged();
		return true;
	}

	/** Module Cooldown **/
	bool IsCooldownActive() const;
	float GetCooldownRemainingSeconds() const;
	void StartCooldown(float DurationSeconds);

	// Add a const getter when a const module instance needs read-only payload access.
	FMASkillPayloadStore& GetPayloadStore() { return PayloadStore; }
	void ResetPayloadStore() { PayloadStore.Reset(); }
	UMASkillRuntimeRegistry* GetRuntimeRegistry() const { return RuntimeRegistry; }

	FMASkillModuleStateChangedSignature OnStateChanged;
	FMASkillSubModulesChangedSignature OnSubModulesChanged;

private:
	void SetInSkillSlot(bool bInSkillSlot);

	UFUNCTION()
	void OnRep_ModuleGroup(FMASkillModuleGroup& PreviousModuleGroup);
	UFUNCTION()
	void OnRep_AddonRuntimeData();
	bool CanModifyAddonRuntimeData() const;
	void NotifyAddonRuntimeDataChanged();
	void InitializePayloadStore();
	void RefreshAddonPayloadMirrors();

	UPROPERTY(ReplicatedUsing=OnRep_ModuleGroup)
	FMASkillModuleGroup ModuleGroup;

	UPROPERTY(Transient)
	FMASkillComposedModule ComposedModule;

	UPROPERTY(Transient)
	bool bIsInSkillSlot = false;

	UPROPERTY(Transient)
	bool bIsActive = true;

	UPROPERTY(Transient)
	FGameplayTag InactiveReasonTag;

	UPROPERTY(ReplicatedUsing=OnRep_AddonRuntimeData)
	FMASkillModuleAddonRuntimeData AddonRuntimeData;

	UPROPERTY(Transient)
	FMASkillPayloadStore PayloadStore;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillRuntimeRegistry> RuntimeRegistry;

	/** Module Cooldown **/
	UFUNCTION()
	void RefreshModuleCooldownState();
	float GetCurrentServerTimeSeconds() const;

	UPROPERTY(ReplicatedUsing=RefreshModuleCooldownState)
	float ModuleCooldownEndTimeSeconds = 0.f;

	FTimerHandle ModuleCooldownTimerHandle;

	friend struct FMASkillAssembler;
	friend class UMAInventoryComponent;
	friend class UMASkillManagerComponent;
};

