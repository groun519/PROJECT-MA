// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Golem/Monster.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

void AMonster::SetGenericTeamId(const FGenericTeamId& NewTeamId)
{
	Super::SetGenericTeamId(NewTeamId);

}

bool AMonster::IsActive() const
{
	return !IsDead();
}

void AMonster::Activate()
{
	RespawnImmediately();
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
