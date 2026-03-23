// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/AIBehaviorTree/BTTask_SendAttackToAbilitySystem.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GAS/MAGameplayAbilityTypes.h"

EBTNodeResult::Type UBTTask_SendAttackToAbilitySystem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
	{
		return EBTNodeResult::Failed;
	}

	APawn* Pawn = AIC->GetPawn();
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return EBTNodeResult::Failed;
	}

	const int32 InputID = static_cast<int32>(EMAAbilityInputID::Attack);
	ASC->PressInputID(InputID);
	ASC->ReleaseInputID(InputID);

	Blackboard->SetValueAsBool(TEXT("ShouldRetreat"), true);

	return EBTNodeResult::Succeeded;
}
