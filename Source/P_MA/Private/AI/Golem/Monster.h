// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/CoinDrop.h"
#include "Character/MACharacter.h"
#include "Monster.generated.h"

/**
 * 
 */
UCLASS()
class AMonster : public AMACharacter
{
	GENERATED_BODY()
	
public:
	AMonster();
	
	DECLARE_MULTICAST_DELEGATE(FOnMonsterDead);
	FOnMonsterDead OnMonsterDead;

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override;

	bool IsActive() const;
	void Activate();
	void SetGoal(AActor* Goal);
	void Deactivate();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float FuryThreshold = 50.f;
	
private:
	virtual void OnRep_TeamID() override;
	virtual void OnDead() override;

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
