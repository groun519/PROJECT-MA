// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTTask_SendSkillToAbilitySystem.h"

#include "AI/Golem/Monster.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GAS/MAGameplayAbilityTypes.h"

UBTTask_SendSkillToAbilitySystem::UBTTask_SendSkillToAbilitySystem()
{
	NodeName = TEXT("Send Skill To Ability System");
	bCreateNodeInstance = true;
	bWaitingForEndEvent = false;

	EndTagContainer.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Monster.Ability.End")));
}

EBTNodeResult::Type UBTTask_SendSkillToAbilitySystem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) return EBTNodeResult::Failed;

	APawn* Pawn = AIC->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!ASC) return EBTNodeResult::Failed;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return EBTNodeResult::Failed;

	RemoveAbilityEndDelegate();

	CachedOwnerComp = &OwnerComp;
	CachedASC = ASC;
	bWaitingForEndEvent = true;

	Blackboard->SetValueAsBool(TEXT("IsUsingSkill"), true);
	Blackboard->SetValueAsBool(TEXT("ShouldRetreat"), false);

	AbilityEndDelegateHandle = ASC->AddGameplayEventTagContainerDelegate(
		EndTagContainer,
		FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(
			this,
			&UBTTask_SendSkillToAbilitySystem::HandleAbilityEnded
		)
	);

	ASC->PressInputID((int32)EMAAbilityInputID::Skill1);

	return EBTNodeResult::InProgress;
}

void UBTTask_SendSkillToAbilitySystem::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	if (bWaitingForEndEvent)
	{
		RemoveAbilityEndDelegate();
		bWaitingForEndEvent = false;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (Blackboard)
	{
		Blackboard->SetValueAsBool(TEXT("IsUsingSkill"), false);
	}

	CachedOwnerComp = nullptr;
	CachedASC = nullptr;

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_SendSkillToAbilitySystem::HandleAbilityEnded(FGameplayTag EventTag, const FGameplayEventData* Payload)
{
	if (!CachedOwnerComp) return;

	UBlackboardComponent* Blackboard = CachedOwnerComp->GetBlackboardComponent();
	if (Blackboard)
	{
		Blackboard->SetValueAsBool(TEXT("IsUsingSkill"), false);
		Blackboard->SetValueAsBool(TEXT("ShouldRetreat"), true);
	}

	RemoveAbilityEndDelegate();
	bWaitingForEndEvent = false;

	FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);

	CachedOwnerComp = nullptr;
	CachedASC = nullptr;
}

void UBTTask_SendSkillToAbilitySystem::RemoveAbilityEndDelegate()
{
	if (CachedASC && AbilityEndDelegateHandle.IsValid())
	{
		CachedASC->RemoveGameplayEventTagContainerDelegate(EndTagContainer, AbilityEndDelegateHandle);
	}

	AbilityEndDelegateHandle.Reset();
}
