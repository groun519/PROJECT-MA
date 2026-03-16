// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EAIStateEnum.h"
#include "AI/MAAIController.h"
#include "BehaviorTree/BTService.h"
#include "BTS_Melee.generated.h"

/**
 * 
 */
UCLASS()
class UBTS_Melee : public UBTService
{
	GENERATED_BODY()

public:
	UBTS_Melee();
	void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	class AMonster* Monster;
	AMAAIController* AIController;
	FVector PlayerLoc;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	EAIStateEnum currentState;
};
