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
}

void UBTS_Melee::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BlackBoard = OwnerComp.GetBlackboardComponent();
	AActor* TargetObj = Cast<AActor>(BlackBoard->GetValueAsObject("Target"));
	AMAPlayerCharacter* TargetCharacter = Cast<AMAPlayerCharacter>(TargetObj);
	Monster = Cast<AMonster>(OwnerComp.GetAIOwner()->GetPawn());
	AIController = Cast<AMAAIController>(OwnerComp.GetAIOwner());

	FVector MonsterLoc = Monster->GetActorLocation();

	if (TargetObj == nullptr)
	{
		currentState = EAIStateEnum::Patrol;
		BlackBoard->SetValueAsEnum("AIState", static_cast<uint8>(currentState));
		return;
	}
	if (TargetCharacter)
	{
		
		PlayerLoc = TargetCharacter->GetActorLocation();
		BlackBoard->SetValueAsVector("PlayerLocation", PlayerLoc);  

		float distance = FVector::Dist(PlayerLoc, MonsterLoc);
		if (distance <= 170.0f)
		{
			currentState = EAIStateEnum::Attack;  
			BlackBoard->SetValueAsEnum("AIState", static_cast<uint8>(currentState)); 
		}
		else
		{
			currentState = EAIStateEnum::Chase;  
			BlackBoard->SetValueAsEnum("AIState", static_cast<uint8>(currentState));  
		}
	}
}
