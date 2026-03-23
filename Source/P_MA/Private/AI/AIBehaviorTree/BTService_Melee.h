// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EAIStateEnum.h"
#include "AI/MAAIController.h"
#include "BehaviorTree/BTService.h"
#include "BTService_Melee.generated.h"

/**
 * 
 */
UCLASS()
class UBTService_Melee : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_Melee();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	void SetAIState(UBlackboardComponent* Blackboard, EAIStateEnum NewState);

public:
	class AMonster* Monster;
	AMAAIController* AIController;
	FVector PlayerLoc;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	EAIStateEnum CurrentState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float StrafeRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float AttackCooldown;

private:
	static const FName TargetKeyName;
	static const FName PlayerLocationKeyName;
	static const FName AIStateKeyName;
	static const FName ShouldRetreatKeyName;
	static const FName AttackBlockedUntilKeyName;
};
