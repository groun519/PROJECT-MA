// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/EQSContext_TargetActor.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "GameFramework/Pawn.h"

void UEQSContext_TargetActor::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	UObject* QuerierObject = QueryInstance.Owner.Get();

	APawn* Pawn = Cast<APawn>(QuerierObject);
	if (!Pawn)
		return;

	AAIController* AIController = Cast<AAIController>(Pawn->GetController());
	if (!AIController)
		return;

	UBlackboardComponent* BB = AIController->GetBlackboardComponent();
	if (!BB)
		return;

	UObject* TargetObject = BB->GetValueAsObject("Target");

	AActor* TargetActor = Cast<AActor>(TargetObject);
	if (!TargetActor)
		return;

	UEnvQueryItemType_Actor::SetContextHelper(ContextData, TargetActor);
}
