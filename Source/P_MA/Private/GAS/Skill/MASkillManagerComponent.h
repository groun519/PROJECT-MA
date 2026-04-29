#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MASkillManagerComponent.generated.h"

class UMASkillAbility;
class UMASkillDefinition;
class UMASkillGenericDataAsset;

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

	UFUNCTION(BlueprintCallable, Category="Skill")
	void ClearDefinitions(EMAAbilityInputID InputID);

	UFUNCTION(BlueprintCallable, Category="Skill")
	void SetSingleDefinition(EMAAbilityInputID InputID, UMASkillDefinition* Definition);

	UFUNCTION(BlueprintCallable, Category="Skill")
	void SetDefinitions(EMAAbilityInputID InputID, const TArray<UMASkillDefinition*>& Definitions);

	UFUNCTION(BlueprintCallable, Category="Skill")
	bool AddDefinition(EMAAbilityInputID InputID, UMASkillDefinition* Definition);

	UFUNCTION(BlueprintCallable, Category="Skill")
	bool InsertDefinition(EMAAbilityInputID InputID, int32 InsertIndex, UMASkillDefinition* Definition);

	UFUNCTION(BlueprintCallable, Category="Skill")
	bool RemoveDefinitionAt(EMAAbilityInputID InputID, int32 RemoveIndex);

	UFUNCTION(BlueprintCallable, Category="Skill")
	bool MoveDefinition(EMAAbilityInputID InputID, int32 FromIndex, int32 ToIndex);

	UFUNCTION(BlueprintPure, Category="Skill")
	int32 GetDefinitionCount(EMAAbilityInputID InputID) const;

	UFUNCTION(BlueprintPure, Category="Skill")
	UMASkillDefinition* GetDefinitionAt(EMAAbilityInputID InputID, int32 Index) const;

	UFUNCTION(BlueprintPure, Category="Skill")
	UMASkillDefinition* GetAssembledDefinition(EMAAbilityInputID InputID) const;
	const UMASkillGenericDataAsset* GetGenericSkillDataAsset() const { return GenericSkillDataAsset; }

	bool RebuildSkill(EMAAbilityInputID InputID);
	void RegisterAbilityHandle(EMAAbilityInputID InputID, FGameplayAbilitySpecHandle AbilityHandle, TSubclassOf<UMASkillAbility> AbilityClass);
	void UnregisterAbilityHandle(EMAAbilityInputID InputID, FGameplayAbilitySpecHandle AbilityHandle);

private:
	FMASkillDefinitionStack* FindStack(EMAAbilityInputID InputID);
	const FMASkillDefinitionStack* FindStack(EMAAbilityInputID InputID) const;
	FMASkillDefinitionStack& FindOrAddStack(EMAAbilityInputID InputID);
	FMASkillSlotStack* FindSkillSlotStack(EMAAbilityInputID InputID);
	const FMASkillSlotStack* FindSkillSlotStack(EMAAbilityInputID InputID) const;
	TArray<EMAAbilityInputID> GatherUniqueSkillSlotInputIDs() const;
	bool CanMutateSkillStacks() const;

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
	TArray<FMASkillSlotStack> ReplicatedSkillSlotStacks;
};
