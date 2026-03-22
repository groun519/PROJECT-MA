// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/BTS_Melee.h"

#include "AI/Golem/Monster.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Player/MAPlayerCharacter.h"

UBTS_Melee::UBTS_Melee()
{
	NodeName = "Player Detect Service";

	Interval = 0.1f;
	RandomDeviation = 0.0f;

	CurrentState = EAIStateEnum::Patrol;

	AttackRange = 170.0f;
	StrafeRange = 500.0f;
	AttackCooldown = 2.0f;
	LastAttackRequestTime = -1000.0f;
}

void UBTS_Melee::SetAIState(UBlackboardComponent* Blackboard, EAIStateEnum NewState)
{
	if (Blackboard == nullptr)
	{
		return;
	}

	switch (NewState)
	{
	case EAIStateEnum::Attack:
		UE_LOG(LogTemp, Warning, TEXT("[BTS] Set AIState = Attack"));
		break;
	case EAIStateEnum::Strafe:
		UE_LOG(LogTemp, Warning, TEXT("[BTS] Set AIState = Strafe"));
		break;
	case EAIStateEnum::Chase:
		UE_LOG(LogTemp, Warning, TEXT("[BTS] Set AIState = Chase"));
		break;
	case EAIStateEnum::Patrol:
		UE_LOG(LogTemp, Warning, TEXT("[BTS] Set AIState = Patrol"));
		break;
	default:
		break;
	}
	
	CurrentState = NewState;
	Blackboard->SetValueAsEnum("AIState", static_cast<uint8>(CurrentState));
}

void UBTS_Melee::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (Blackboard == nullptr)
	{
		return;
	}

	AActor* TargetObj = Cast<AActor>(Blackboard->GetValueAsObject("Target"));
	AMAPlayerCharacter* TargetCharacter = Cast<AMAPlayerCharacter>(TargetObj);
	AIController = Cast<AMAAIController>(OwnerComp.GetAIOwner());
	Monster = Cast<AMonster>(OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr);

	if (Monster == nullptr)
	{
		return;
	}

	const FVector MonsterLoc = Monster->GetActorLocation();

	if (TargetObj == nullptr || TargetCharacter == nullptr)
	{
		SetAIState(Blackboard, EAIStateEnum::Patrol);
		return;
	}

	PlayerLoc = TargetCharacter->GetActorLocation();
	Blackboard->SetValueAsVector("PlayerLocation", PlayerLoc);

	const float Distance = FVector::Dist(PlayerLoc, MonsterLoc);
	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const bool bAttackReady = (CurrentTime - LastAttackRequestTime) >= AttackCooldown;

	if (Distance <= AttackRange)
	{
		SetAIState(Blackboard, EAIStateEnum::Attack);
		return;
	}

	if (Distance <= StrafeRange)
	{
		SetAIState(Blackboard, EAIStateEnum::Strafe);
		return;
	}

	SetAIState(Blackboard, EAIStateEnum::Chase);
}

