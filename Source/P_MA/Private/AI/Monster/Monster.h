#pragma once

#include "CoreMinimal.h"
#include "AI/Monster/MAMonsterTypes.h"
#include "Character/MACharacter.h"
#include "Monster.generated.h"

class UStateTree;

USTRUCT(BlueprintType)
struct FMonsterEnvData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Env")
	FGameplayTag EnvTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Env")
	TArray<UMaterialInterface*> MIList;
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

	bool IsActive() const;
	void Activate();
	void SetGoal(AActor* Goal);
	void Deactivate();
	
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
	
	UPROPERTY()
	bool bActiveInPool = true;
	
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName GoalBlackboardKeyName = "Goal";

	FTimerHandle DisappearTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category="Death")
	float DisappearDelay = 3.f;
};
