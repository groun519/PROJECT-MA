// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/BTS_SetFocus.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTS_SetFocus::UBTS_SetFocus()
{
	NodeName = "Set Focus";
	bNotifyTick = true;
}

void UBTS_SetFocus::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController == nullptr)
	{
		return;
	}
	UBlackboardComponent* BlackBoard = AIController->GetBlackboardComponent();

	AActor* TargetActor = Cast<AActor>(BlackBoard->GetValueAsObject("Target"));
	if (TargetActor)
	{
		FVector TargetLocation = TargetActor->GetActorLocation();
		AIController->SetFocalPoint(TargetLocation);
	}
}
