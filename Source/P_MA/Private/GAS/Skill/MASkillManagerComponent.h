#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MASkillManagerComponent.generated.h"

class UMASkillAbility;
class UMASkillDefinition;
class UMASkillGenericDataAsset;
class UMASkillModuleInstance;
class UActorChannel;
class FOutBunch;
struct FReplicationFlags;

DECLARE_MULTICAST_DELEGATE_OneParam(FMASkillSlotChangedSignature, EMAAbilityInputID);

USTRUCT()
struct FMASkillDefinitionStack
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	EMAAbilityInputID InputID = EMAAbilityInputID::None;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMASkillModuleInstance>> SourceModuleInstances;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillDefinition> AssembledDefinition = nullptr;

	UPROPERTY(Transient)
	FGameplayAbilitySpecHandle AbilityHandle;
};

USTRUCT(BlueprintType)
struct FMASkillSlotStack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill")
	EMAAbilityInputID InputID = EMAAbilityInputID::None;
};

USTRUCT()
struct FMASkillReplicatedSlotStack
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	EMAAbilityInputID InputID = EMAAbilityInputID::None;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMASkillModuleInstance>> ModuleInstances;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MA_API UMASkillManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMASkillManagerComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	void InitializeGrantedAbilities();
	void PrepareSkillSlotStacksForUI();

	FMASkillSlotChangedSignature OnSkillSlotChanged;

	bool ReplaceDefinitionAt(
		EMAAbilityInputID InputID,
		int32 DefinitionIndex,
		UMASkillDefinition* NewDefinition,
		UMASkillDefinition*& OutPreviousDefinition);
	bool ReplaceModuleInstanceAt(
		EMAAbilityInputID InputID,
		int32 ModuleIndex,
		UMASkillModuleInstance* NewModuleInstance,
		UMASkillModuleInstance*& OutPreviousModuleInstance);

	bool RequestSwapDefinitionSlotsBetween(
		EMAAbilityInputID InputIDA,
		int32 IndexA,
		EMAAbilityInputID InputIDB,
		int32 IndexB);
	bool RequestMoveDefinitionSlot(
		const TArray<TObjectPtr<UMASkillModuleInstance>>* SourceSlots,
		int32 SourceIndex,
		UActorComponent* TargetOwner,
		const TArray<TObjectPtr<UMASkillModuleInstance>>* TargetSlots,
		int32 TargetIndex);

	const TArray<TObjectPtr<UMASkillModuleInstance>>* GetDefinitionSlotsForUI(EMAAbilityInputID InputID);
	bool FindInputIDForDefinitionSlots(const TArray<TObjectPtr<UMASkillModuleInstance>>* DefinitionSlots, EMAAbilityInputID& OutInputID) const;

	TArray<EMAAbilityInputID> GetSkillSlotInputIDs() const
	{
		return GatherUniqueSkillSlotInputIDs();
	}

	UMASkillDefinition* GetAssembledDefinition(EMAAbilityInputID InputID) const;
	const UMASkillGenericDataAsset* GetGenericSkillDataAsset() const { return GenericSkillDataAsset; }

	bool RebuildSkill(EMAAbilityInputID InputID);
	void RegisterAbilityHandle(EMAAbilityInputID InputID, FGameplayAbilitySpecHandle AbilityHandle, TSubclassOf<UMASkillAbility> AbilityClass);
	void UnregisterAbilityHandle(EMAAbilityInputID InputID, FGameplayAbilitySpecHandle AbilityHandle);

private:
	static constexpr int32 SkillModuleSlotCount = 8;

	FMASkillDefinitionStack* FindStack(EMAAbilityInputID InputID);
	const FMASkillDefinitionStack* FindStack(EMAAbilityInputID InputID) const;
	FMASkillDefinitionStack& FindOrAddStack(EMAAbilityInputID InputID);
	FMASkillSlotStack* FindSkillSlotStack(EMAAbilityInputID InputID);
	const FMASkillSlotStack* FindSkillSlotStack(EMAAbilityInputID InputID) const;
	bool IsConfiguredSkillSlotInputID(EMAAbilityInputID InputID) const;
	static bool IsValidDefinitionSlotIndex(int32 Index);
	static void NormalizeModuleInstanceSlots(TArray<TObjectPtr<UMASkillModuleInstance>>& ModuleInstances);
	static bool HasAnyModuleInstance(const TArray<TObjectPtr<UMASkillModuleInstance>>& ModuleInstances);
	TArray<EMAAbilityInputID> GatherUniqueSkillSlotInputIDs() const;
	bool CanMutateSkillStacks() const;
	bool SwapDefinitionSlotsBetween(
		EMAAbilityInputID InputIDA,
		int32 IndexA,
		EMAAbilityInputID InputIDB,
		int32 IndexB);

	UFUNCTION(Server, Reliable)
	void ServerSwapDefinitionSlotsBetween(
		EMAAbilityInputID InputIDA,
		int32 IndexA,
		EMAAbilityInputID InputIDB,
		int32 IndexB);

	UFUNCTION()
	void OnRep_ReplicatedSkillSlotStacks();

	void ApplyReplicatedSkillSlotStacks();
	void UpdateReplicatedSkillSlotStack(const FMASkillDefinitionStack& SkillStack);
	void RefreshAbilityDefinition(FMASkillDefinitionStack& SkillStack);
	UMASkillAbility* ResolveSkillAbility(const FMASkillDefinitionStack& SkillStack) const;

	UPROPERTY(Transient)
	TArray<FMASkillDefinitionStack> SkillStacks;

	UPROPERTY(EditDefaultsOnly, Category="Skill")
	TArray<FMASkillSlotStack> SkillSlotStacks;

	UPROPERTY(EditDefaultsOnly, Category="Skill")
	TObjectPtr<UMASkillGenericDataAsset> GenericSkillDataAsset;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_ReplicatedSkillSlotStacks)
	TArray<FMASkillReplicatedSlotStack> ReplicatedSkillSlotStacks;
};
