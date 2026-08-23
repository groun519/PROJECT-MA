#pragma once

#include "CoreMinimal.h"
#include "AI/Monster/MAMonsterTypes.h"
#include "Character/MACharacter.h"
#include "Monster.generated.h"

class UStateTree;
class UMASkillModule;
class UMASkillModulePool;

USTRUCT(BlueprintType)
struct FMonsterEnvData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Env")
	FGameplayTag EnvTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Env")
	TArray<UMaterialInterface*> MIList;
};

USTRUCT(BlueprintType)
struct FMAModuleDropRoll
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Drop", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ChancePerRoll = 1.f;

	UPROPERTY(EditAnywhere, Category="Drop", meta=(ClampMin="1"))
	int32 RollCount = 1;
};

UCLASS()
class AMonster : public AMACharacter
{
	GENERATED_BODY()

public:
	AMonster(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnDead() override;
	virtual void PossessedBy(AController* NewController) override;

public:
	DECLARE_MULTICAST_DELEGATE(FOnMonsterDead);
	FOnMonsterDead OnMonsterDead;

	void SetGoal(AActor* Goal);
	
	void SetEnvTag(const FGameplayTag& InEnvTag);
	void SetStatCoefficient(float InCoefficient) { StatCoefficient = InCoefficient; }
	bool SelectWeightedSkill();
	bool SetPatternPlanFromStateNames(const TArray<FName>& StateNames);
	bool HasPendingPatternPlan() const { return !PatternPlan.IsEmpty(); }
	bool SelectNextPatternPlanFragment();
	void ResetSkillSelection();
	bool HasSelectedSkill() const { return SelectedSkillSlotTag.IsValid(); }
	FGameplayTag GetSelectedSkillSlotTag() const { return SelectedSkillSlotTag; }
	float GetSelectedSkillUseDistance() const { return SelectedSkillUseDistance; }
	const UStateTree* GetPatternStateTree() const { return PatternStateTree; }
	
private:
	void ApplyStatCoefficientEffect();
	void ApplyEnvMaterials();
	void InitializeSkills();
	void TrySpawnModuleDrops();
	static bool LoadSkillModules(
		const TArray<TSoftObjectPtr<UMASkillModule>>& ModuleAssets,
		TArray<TObjectPtr<UMASkillModule>>& OutModules);
	bool ApplyPatternRowToActiveSlot(const FMonsterSkillPatternRow& PatternRow);
	static const FMonsterSkillPatternRow* ResolvePatternRow(const UDataTable* PatternDataTable, FName RowName, const TCHAR* Context);

	UPROPERTY(ReplicatedUsing=OnRep_EnvGameplayTag, EditAnywhere, Category = "Env")
	FGameplayTag EnvGameplayTag;

	UFUNCTION()
	void OnRep_EnvGameplayTag();

	UPROPERTY(EditDefaultsOnly, Category = "Env")
	TArray<FMonsterEnvData> EnvTagToMaterial;

	UPROPERTY()
	float StatCoefficient = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TArray<FMonsterSkillSlotData> SkillSlots;

	UPROPERTY(EditDefaultsOnly, Category = "Pattern", meta=(Categories="Skill.Slot.Active"))
	FGameplayTag PatternSlotTag = FGameplayTag::RequestGameplayTag(TEXT("Skill.Slot.Active.NoCooldown"));

	UPROPERTY(EditDefaultsOnly, Category = "Pattern")
	TObjectPtr<UStateTree> PatternStateTree;

	UPROPERTY(EditDefaultsOnly, Category = "Pattern")
	TObjectPtr<UDataTable> PatternDataTable;

	UPROPERTY()
	FGameplayTag SelectedSkillSlotTag;

	UPROPERTY()
	float SelectedSkillUseDistance = 0.f;

	UPROPERTY(Transient)
	TArray<FName> PatternPlan;
	
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName GoalBlackboardKeyName = "Goal";

	UPROPERTY(EditDefaultsOnly, Category="Death")
	float DisappearDelay = 3.f;

	UPROPERTY(EditDefaultsOnly, Category="Drop")
	TMap<TObjectPtr<UMASkillModulePool>, FMAModuleDropRoll> ModuleDropPools;
};
