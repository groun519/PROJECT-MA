// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/BTS_Melee.h"

#include "AI/Golem/Monster.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Player/MAPlayerCharacter.h"

const FName UBTS_Melee::TargetKeyName(TEXT("Target"));
const FName UBTS_Melee::PlayerLocationKeyName(TEXT("PlayerLocation"));
const FName UBTS_Melee::AIStateKeyName(TEXT("AIState"));
const FName UBTS_Melee::ShouldRetreatKeyName(TEXT("ShouldRetreat"));

UBTS_Melee::UBTS_Melee()
{
	NodeName = "Player Detect Service";

	Interval = 0.1f;
	RandomDeviation = 0.0f;

	CurrentState = EAIStateEnum::Patrol;

	AttackRange = 170.0f;
	StrafeRange = 500.0f;
	AttackCooldown = 2.0f;

	Monster = nullptr;
	AIController = nullptr;
	PlayerLoc = FVector::ZeroVector;
}

void UBTS_Melee::SetAIState(UBlackboardComponent* Blackboard, EAIStateEnum NewState)
{
	if (Blackboard == nullptr)
	{
		return;
	}

	if (CurrentState == NewState)
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

	case EAIStateEnum::Retreat:
		UE_LOG(LogTemp, Warning, TEXT("[BTS] Set AIState = Retreat"));
		break;

	case EAIStateEnum::Patrol:
		UE_LOG(LogTemp, Warning, TEXT("[BTS] Set AIState = Patrol"));
		break;

	default:
		break;
	}

	CurrentState = NewState;
	Blackboard->SetValueAsEnum(AIStateKeyName, static_cast<uint8>(CurrentState));
}

void UBTS_Melee::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (Blackboard == nullptr)
	{
		return;
	}

	const bool bShouldRetreat = Blackboard->GetValueAsBool(ShouldRetreatKeyName);
	if (bShouldRetreat)
	{
		SetAIState(Blackboard, EAIStateEnum::Retreat);
		return;
	}

	AActor* TargetObj = Cast<AActor>(Blackboard->GetValueAsObject(TargetKeyName));
	AMAPlayerCharacter* TargetCharacter = Cast<AMAPlayerCharacter>(TargetObj);

	AIController = Cast<AMAAIController>(OwnerComp.GetAIOwner());
	Monster = Cast<AMonster>(OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr);

	if (Monster == nullptr)
	{
		return;
	}

	if (TargetObj == nullptr || TargetCharacter == nullptr)
	{
		SetAIState(Blackboard, EAIStateEnum::Patrol);
		return;
	}

	const FVector MonsterLoc = Monster->GetActorLocation();
	PlayerLoc = TargetCharacter->GetActorLocation();

	Blackboard->SetValueAsVector(PlayerLocationKeyName, PlayerLoc);

	const float Distance = FVector::Dist(PlayerLoc, MonsterLoc);
	
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

