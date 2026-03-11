// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/CoinDrop.h"
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

/**
 * 
 */
UCLASS()
class AMonster : public AMACharacter
{
	GENERATED_BODY()
	
public:
	AMonster();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	DECLARE_MULTICAST_DELEGATE(FOnMonsterDead);
	FOnMonsterDead OnMonsterDead;

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override;

	bool IsActive() const;
	void Activate();
	void SetGoal(AActor* Goal);
	void Deactivate();
	
	void SetEnvTag(const FGameplayTag& InEnvTag);
	void SetDropGold(int32 InGold)
	{
		DropGold = InGold;
	};
	void SetStatCoefficient(float InCoefficient)
	{
		StatCoefficient = InCoefficient;
	};
	void ApplyEnvMaterials();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	bool bUseFuryThreshold = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float FuryThreshold = 50.f;
	
protected:
	virtual void BeginPlay() override;

private:
	virtual void OnRep_TeamID() override;
	virtual void OnDead() override;

	UPROPERTY(ReplicatedUsing=OnRep_EnvGameplayTag, EditAnywhere, Category = "Env")
	FGameplayTag EnvGameplayTag;

	UFUNCTION()
	void OnRep_EnvGameplayTag();

	UPROPERTY(EditDefaultsOnly, Category = "Env")
	TArray<FMonsterEnvData> EnvTagToMaterial;

	// 죽었을 때 줄 골드량
	UPROPERTY()
	int32 DropGold = 0;

	// 생성될 때 곱해질 스테이터스 계수
	UPROPERTY()
	float StatCoefficient = 1.f;
	
	UPROPERTY()
	bool bActiveInPool = true;
	
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName GoalBlackboardKeyName = "Goal";

	FTimerHandle DisappearTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category="Death")
	float DisappearDelay = 3.f;

	UPROPERTY(VisibleAnywhere)
	UCoinDrop* CoinDropComp;
};
