// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AIBehaviorTree/BTService_SetFocus.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_SetFocus::UBTService_SetFocus()
{
	NodeName = "Set Focus";
	bNotifyTick = true;
}

void UBTService_SetFocus::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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
