// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Golem/Monster.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void AMonster::SetGenericTeamId(const FGenericTeamId& NewTeamId)
{
	Super::SetGenericTeamId(NewTeamId);
}

bool AMonster::IsActive() const
{
	return bActiveInPool && !IsDead();
}

void AMonster::Activate()
{
	bActiveInPool = true;

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}

	// RespawnImmediately()는 체력 등 초기화용
	RespawnImmediately();

	// ★ AI 다시 시작 (필수)
	if (AController* BaseCon = GetController())
	{
		if (AAIController* AICon = Cast<AAIController>(BaseCon))
		{
			if (UBrainComponent* Brain = AICon->GetBrainComponent())
			{
				Brain->StartLogic();           // ← 이거 없으면 절대 안 움직임
			}
		}
	}
}

void AMonster::Deactivate()
{
	bActiveInPool = false;

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}

	if (AController* BaseCon = GetController())
	{
		if (AAIController* AICon = Cast<AAIController>(BaseCon))
		{
			if (UBrainComponent* Brain = AICon->GetBrainComponent())
			{
				Brain->StopLogic(TEXT("Monster Deactivated"));
			}
		}
	}
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

void AMonster::OnDead()
{
	OnMonsterDead.Broadcast();
}
