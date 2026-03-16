// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/BTTask_MoveToPlayer.h"

#include "EAIStateEnum.h"
#include "AI/Golem/Monster.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_MoveToPlayer::UBTTask_MoveToPlayer()
{
	NodeName = TEXT("Move To Target Player");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_MoveToPlayer::ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory)
{
	Super::ExecuteTask(ownerComp, nodeMemory);

	BlackboardComp = ownerComp.GetBlackboardComponent();
	OwnerComp = &ownerComp;
	AIController = Cast<AMAAIController>(ownerComp.GetAIOwner());
	UObject* targetObject = BlackboardComp->GetValueAsObject(GetSelectedBlackboardKey());
	
	TargetActor = Cast<AActor>(targetObject);
    
	if (!TargetActor || !AIController)
	{
		return EBTNodeResult::Failed;
	}
	return EBTNodeResult::InProgress;
}

void UBTTask_MoveToPlayer::TickTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory, float deltaSeconds)
{
	Super::TickTask(ownerComp, nodeMemory, deltaSeconds);

	CurrentState = (EAIStateEnum)BlackboardComp->GetValueAsEnum(TEXT("AIState"));
	if (CurrentState != EAIStateEnum::Chase)
	{
		AIController->StopMovement();
		FinishLatentTask(ownerComp, EBTNodeResult::Failed);
		return;
	}
	if (TargetActor)
	{
		FVector currentTargetLoc = TargetActor->GetActorLocation();
		AIController->MoveToLocation(currentTargetLoc);
	}
}
