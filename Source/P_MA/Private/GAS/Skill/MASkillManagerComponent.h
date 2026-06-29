#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GAS/Skill/MASkillSystemTypes.h"
#include "MASkillManagerComponent.generated.h"

class UMASkillAbility;
class UMASkillDefinition;
class UMASkillEventDispatcher;
class UMASkillEventRouter;
class UMASkillModuleInstance;
class UActorChannel;
class FOutBunch;
struct FMASkillEvent;
struct FReplicationFlags;

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
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	void InitializeGrantedAbilities();
	void PrepareSkillSlotRuntimeStatesForUI();

	FMASkillSlotChangedSignature OnSkillSlotChanged;

	bool ReplaceDefinitionAt(
		FGameplayTag SlotTag,
		int32 ModuleIndex,
		UMASkillDefinition* NewDefinition);
	bool ReplaceDefinitionsAt(
		FGameplayTag SlotTag,
		const TArray<TObjectPtr<UMASkillDefinition>>& NewDefinitions);
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
	FGameplayTag GetActivePreviewElementalTag() const { return ActivePreviewElementalTag; }

	TArray<FGameplayTag> GetSkillSlotTags() const
	{
		return GatherUniqueSkillSlotTags();
	}

	UMASkillDefinition* GetAssembledDefinition(FGameplayTag SlotTag) const;

	bool RebuildSkill(FGameplayTag SlotTag);
	void RegisterAbilityHandle(FGameplayTag SlotTag, FGameplayAbilitySpecHandle AbilityHandle, TSubclassOf<UMASkillAbility> AbilityClass);
	void UnregisterAbilityHandle(FGameplayTag SlotTag, FGameplayAbilitySpecHandle AbilityHandle);
	void ClearActivePreviewElementalTag();
	bool TryActivateSkill(FGameplayTag SlotTag);
	UMASkillAbility* GetSkillAbility(FGameplayTag SlotTag) const;
	UMASkillEventRouter* GetEventRouter() const { return Router; }
	UMASkillEventDispatcher* GetEventDispatcher() const { return Dispatcher; }

private:
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
	bool EnsureAbilityForSlot(FMASkillSlotRuntimeState& SlotState);
	void SetActivePreviewElementalTagFromSlot(const FMASkillSlotRuntimeState& SlotState);

	UFUNCTION(Server, Reliable)
	void ServerSwapModuleSlotsBetween(
		FGameplayTag SlotTagA,
		int32 IndexA,
		FGameplayTag SlotTagB,
		int32 IndexB);

	UFUNCTION()
	void OnRep_ReplicatedSkillSlotRuntimeStates();

	void ApplyReplicatedSkillSlotRuntimeStates();
	void UpdateReplicatedSkillSlotRuntimeState(const FMASkillSlotRuntimeState& SlotState);
	void RefreshAbilityDefinition(FMASkillSlotRuntimeState& SlotState);
	UMASkillAbility* ResolveSkillAbility(const FMASkillSlotRuntimeState& SlotState) const;

	UPROPERTY(Transient)
	TArray<FMASkillSlotRuntimeState> SkillSlotRuntimeStates;

	UPROPERTY(EditDefaultsOnly, Category="Skill")
	TArray<FMASkillSlotStack> SkillSlotStacks;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_ReplicatedSkillSlotRuntimeStates)
	TArray<FMASkillReplicatedSlotRuntimeState> ReplicatedSkillSlotRuntimeStates;

	UPROPERTY(Transient, Replicated)
	FGameplayTag ActivePreviewElementalTag;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillEventRouter> Router;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillEventDispatcher> Dispatcher;
};
