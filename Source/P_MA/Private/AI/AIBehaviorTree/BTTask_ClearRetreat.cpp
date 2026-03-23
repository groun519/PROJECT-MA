// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/BTTask_ClearRetreat.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ClearRetreat::UBTTask_ClearRetreat()
{
	NodeName = "Clear Retreat Flag";
	StrafeAfterRetreatTime = 2.0f;
}

EBTNodeResult::Type UBTTask_ClearRetreat::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (Blackboard == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AAIController* AIC = OwnerComp.GetAIOwner();
	if (AIC == nullptr || AIC->GetPawn() == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	const float CurrentTime = AIC->GetPawn()->GetWorld()->GetTimeSeconds();
	Blackboard->SetValueAsFloat(TEXT("AttackBlockedUntil"), CurrentTime + StrafeAfterRetreatTime);
	Blackboard->SetValueAsBool(TEXT("ShouldRetreat"), false);

	return EBTNodeResult::Succeeded;
}
