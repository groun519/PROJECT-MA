// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/BTTask_ClearRetreat.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ClearRetreat::UBTTask_ClearRetreat()
{
	NodeName = "Clear Retreat Flag";
}

EBTNodeResult::Type UBTTask_ClearRetreat::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (Blackboard == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	Blackboard->SetValueAsBool(TEXT("ShouldRetreat"), false);

	return EBTNodeResult::Succeeded;
}
