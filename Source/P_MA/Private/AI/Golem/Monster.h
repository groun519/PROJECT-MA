#pragma once

#include "CoreMinimal.h"
#include "AI/MAMonsterTypes.h"
#include "Character/MACharacter.h"
#include "Monster.generated.h"

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
	AMonster();

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
	void SetSkillSlots(const TArray<FMonsterSkillSlotData>& InSkillSlots) { SkillSlots = InSkillSlots; }
	bool SelectWeightedSkill();
	bool HasSelectedSkill() const { return SelectedSkillSlotTag.IsValid(); }
	FGameplayTag GetSelectedSkillSlotTag() const { return SelectedSkillSlotTag; }
	float GetSelectedSkillUseDistance() const { return SelectedSkillUseDistance; }
	
private:
	void ApplyStatCoefficientEffect();
	void ApplyEnvMaterials();
	void InitializeSkills();

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

	UPROPERTY()
	FGameplayTag SelectedSkillSlotTag;

	UPROPERTY()
	float SelectedSkillUseDistance = 0.f;
	
	UPROPERTY()
	bool bActiveInPool = true;
	
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName GoalBlackboardKeyName = "Goal";

	FTimerHandle DisappearTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category="Death")
	float DisappearDelay = 3.f;
};
