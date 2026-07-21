#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GAS/Skill/Area/MASkillAreaTypes.h"
#include "GAS/Skill/MASkillSystemTypes.h"
#include "MASkillManagerComponent.generated.h"

class UMASkillAbility;
class UMASkillEventDispatcher;
class UMASkillEventRouter;
class UMASkillModule;
class UMASkillModuleInstance;
class UAnimSequenceBase;
struct FMASkillEvent;

DECLARE_MULTICAST_DELEGATE_OneParam(FMASkillSlotChangedSignature, FGameplayTag);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MA_API UMASkillManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMASkillManagerComponent();
	virtual void InitializeComponent() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void InitializeGrantedAbilities();
	void PrepareSkillSlotRuntimeStatesForUI();

	FMASkillSlotChangedSignature OnSkillSlotChanged;

	/** Module Lifetime **/
	UMASkillModuleInstance* CreateModuleInstance(UMASkillModule* Module);

	/** Slot Composition **/
	bool ReplaceModuleAt(
		FGameplayTag SlotTag,
		int32 ModuleIndex,
		UMASkillModule* NewModule);
	bool ReplaceModulesAt(
		FGameplayTag SlotTag,
		const TArray<TObjectPtr<UMASkillModule>>& NewModules);
	bool ReplaceModuleInstanceAt(
		FGameplayTag SlotTag,
		int32 ModuleIndex,
		UMASkillModuleInstance* NewModuleInstance,
		UMASkillModuleInstance*& OutPreviousModuleInstance);

	bool RequestSwapModuleSlotsBetween(
		FGameplayTag SlotTagA,
		int32 IndexA,
		FGameplayTag SlotTagB,
		int32 IndexB);
	bool RequestMoveModuleSlot(
		const TArray<TObjectPtr<UMASkillModuleInstance>>* SourceSlots,
		int32 SourceIndex,
		UActorComponent* TargetOwner,
		const TArray<TObjectPtr<UMASkillModuleInstance>>* TargetSlots,
		int32 TargetIndex);

	const TArray<TObjectPtr<UMASkillModuleInstance>>* GetModuleSlotsForUI(FGameplayTag SlotTag);
	bool FindSlotTagForModuleSlots(const TArray<TObjectPtr<UMASkillModuleInstance>>* ModuleSlots, FGameplayTag& OutSlotTag) const;
	TArray<FGameplayTag> GetSkillSlotTags() const
	{
		return GatherUniqueSkillSlotTags();
	}

	/** Slot Runtime **/
	FGameplayTag GetActivePreviewVisualElementTag() const { return ActivePreviewVisualElementTag; }
	UMASkillModule* GetAssembledModule(FGameplayTag SlotTag) const;

	void RebuildSkill(FGameplayTag SlotTag);
	void RegisterAbilityHandle(FGameplayTag SlotTag, FGameplayAbilitySpecHandle AbilityHandle, TSubclassOf<UMASkillAbility> AbilityClass);
	void UnregisterAbilityHandle(FGameplayTag SlotTag, FGameplayAbilitySpecHandle AbilityHandle);
	void ClearActivePreviewVisualElementTag();
	bool TryActivateSkill(FGameplayTag SlotTag);
	UMASkillAbility* GetSkillAbility(FGameplayTag SlotTag) const;
	UMASkillEventRouter* GetEventRouter() const { return Router; }
	UMASkillEventDispatcher* GetEventDispatcher() const { return Dispatcher; }

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnSkillAreaImpact(FMASkillWorldAreaShape Area, FGameplayTag VisualTag);
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnPredictedSkillAreaImpact(FMASkillWorldAreaShape Area, FGameplayTag VisualTag);
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_RegisterSkillAreaPreviewContext(
		UAnimSequenceBase* Animation,
		float ResolvedAreaScale,
		FGameplayTag VisualTag);
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_UnregisterSkillAreaPreviewContext(UAnimSequenceBase* Animation);

private:
	/** Module Lifetime **/
	void UnregisterModuleInstance(UMASkillModuleInstance* ModuleInstance);

	/** Slot Composition **/
	static constexpr int32 ActiveModuleSlotCount = 8;
	static constexpr int32 PassiveModuleSlotCount = 4;

	FMASkillSlotRuntimeState* FindSlotRuntimeState(FGameplayTag SlotTag);
	const FMASkillSlotRuntimeState* FindSlotRuntimeState(FGameplayTag SlotTag) const;
	FMASkillSlotRuntimeState& FindOrAddSlotRuntimeState(FGameplayTag SlotTag);
	static int32 GetModuleSlotCount(FGameplayTag SlotTag);
	static bool IsValidModuleSlotIndex(FGameplayTag SlotTag, int32 Index);
	static void NormalizeModuleInstanceSlots(FGameplayTag SlotTag, TArray<TObjectPtr<UMASkillModuleInstance>>& ModuleInstances);
	TArray<FGameplayTag> GatherUniqueSkillSlotTags() const;
	bool CanMutateSkillSlots() const;
	bool SwapModuleSlotsBetween(
		FGameplayTag SlotTagA,
		int32 IndexA,
		FGameplayTag SlotTagB,
		int32 IndexB);

	UFUNCTION(Server, Reliable)
	void ServerSwapModuleSlotsBetween(
		FGameplayTag SlotTagA,
		int32 IndexA,
		FGameplayTag SlotTagB,
		int32 IndexB);

	/** Slot Replication **/
	UFUNCTION()
	void OnRep_ReplicatedSkillSlotRuntimeStates();

	void ApplyReplicatedSkillSlotRuntimeStates();
	void UpdateReplicatedSkillSlotRuntimeState(const FMASkillSlotRuntimeState& SlotState);

	/** Slot Runtime **/
	bool EnsureAbilityForSlot(FMASkillSlotRuntimeState& SlotState);
	void RefreshAbilityModule(FMASkillSlotRuntimeState& SlotState);
	UMASkillAbility* ResolveSkillAbility(const FMASkillSlotRuntimeState& SlotState) const;
	void SetActivePreviewVisualElementTagFromSlot(const FMASkillSlotRuntimeState& SlotState);
	void NotifyActiveModulesChanged(const FMASkillSlotRuntimeState& SlotState);

	void SpawnSkillAreaImpactLocal(
		const FMASkillWorldAreaShape& Area,
		FGameplayTag VisualTag,
		bool bSkipAutonomousProxy);

	UPROPERTY(Transient)
	TArray<FMASkillSlotRuntimeState> SkillSlotRuntimeStates;

	UPROPERTY(EditDefaultsOnly, Category="Skill")
	TArray<FMASkillSlotStack> SkillSlotStacks;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_ReplicatedSkillSlotRuntimeStates)
	TArray<FMASkillReplicatedSlotRuntimeState> ReplicatedSkillSlotRuntimeStates;

	UPROPERTY(Transient, Replicated)
	FGameplayTag ActivePreviewVisualElementTag;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillEventRouter> Router;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillEventDispatcher> Dispatcher;
};
