#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MASkillManagerComponent.generated.h"

class UMASkillAbility;
class UMASkillDefinition;
class UMASkillGenericDataAsset;

DECLARE_MULTICAST_DELEGATE_OneParam(FMASkillSlotChangedSignature, EMAAbilityInputID);

USTRUCT(BlueprintType)
struct FMASkillDefinitionStack
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Skill")
	EMAAbilityInputID InputID = EMAAbilityInputID::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Skill")
	TArray<TObjectPtr<UMASkillDefinition>> SourceDefinitions;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill")
	TArray<TObjectPtr<UMASkillDefinition>> Definitions;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MA_API UMASkillManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMASkillManagerComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void InitializeGrantedAbilities();
	void PrepareSkillSlotStacksForUI();

	FMASkillSlotChangedSignature OnSkillSlotChanged;

	bool AddDefinition(EMAAbilityInputID InputID, UMASkillDefinition* Definition);

	bool ReplaceDefinitionAt(
		EMAAbilityInputID InputID,
		int32 DefinitionIndex,
		UMASkillDefinition* NewDefinition,
		UMASkillDefinition*& OutPreviousDefinition);

	bool RequestSwapDefinitionSlotsBetween(
		EMAAbilityInputID InputIDA,
		int32 IndexA,
		EMAAbilityInputID InputIDB,
		int32 IndexB);
	bool RequestMoveDefinitionSlot(
		const TArray<TObjectPtr<UMASkillDefinition>>* SourceSlots,
		int32 SourceIndex,
		UActorComponent* TargetOwner,
		const TArray<TObjectPtr<UMASkillDefinition>>* TargetSlots,
		int32 TargetIndex);

	const TArray<TObjectPtr<UMASkillDefinition>>* GetDefinitionSlotsForUI(EMAAbilityInputID InputID);
	bool FindInputIDForDefinitionSlots(const TArray<TObjectPtr<UMASkillDefinition>>* DefinitionSlots, EMAAbilityInputID& OutInputID) const;

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
	static void NormalizeDefinitionSlots(TArray<TObjectPtr<UMASkillDefinition>>& Definitions);
	static void CopyDefinitionSlots(TArray<TObjectPtr<UMASkillDefinition>>& Target, const TArray<TObjectPtr<UMASkillDefinition>>& Source);
	static bool HasAnyDefinition(const TArray<TObjectPtr<UMASkillDefinition>>& Definitions);
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

	// TODO: This is an editor/test seed path. Replace with explicit runtime seeding from saved data/loadout before removing fallback reads.
	UPROPERTY(EditDefaultsOnly, Category="Skill")
	TArray<FMASkillSlotStack> SkillSlotStacks;

	UPROPERTY(EditDefaultsOnly, Category="Skill")
	TObjectPtr<UMASkillGenericDataAsset> GenericSkillDataAsset;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_ReplicatedSkillSlotStacks)
	TArray<FMASkillSlotStack> ReplicatedSkillSlotStacks;
};
