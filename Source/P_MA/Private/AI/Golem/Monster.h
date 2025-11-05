// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override;

	bool IsActive() const;
	void Activate();
	void SetGoal(AActor* Goal);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float FuryThreshold = 50.f;
	
private:
	virtual void OnRep_TeamID() override;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName GoalBlackboardKeyName = "Goal";
};
