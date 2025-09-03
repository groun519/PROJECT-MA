// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Golem/Monster.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GAS/MAAbilitySystemStatics.h"

void AMonster::SetGenericTeamId(const FGenericTeamId& NewTeamId)
{
	Super::SetGenericTeamId(NewTeamId);

}

bool AMonster::IsActive() const
{
	return !GetAbilitySystemComponent()->HasMatchingGameplayTag(UMAAbilitySystemStatics::GetDeadStatTag());
}

void AMonster::Activate()
{
	GetAbilitySystemComponent()->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(UMAAbilitySystemStatics::GetDeadStatTag()));
}

void AMonster::SetGoal(AActor* Goal)
{
	if (AAIController* AIController = GetController<AAIController>())
	{
		if (UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent())
		{
			BlackboardComponent->SetValueAsObject(GoalBlackboardKeyName, Goal);
		}
	}
}

void AMonster::OnRep_TeamID()
{

}
