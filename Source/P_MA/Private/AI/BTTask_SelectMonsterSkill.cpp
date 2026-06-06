#include "AI/BTTask_SelectMonsterSkill.h"

#include "AIController.h"
#include "AI/Golem/Monster.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_SelectMonsterSkill::UBTTask_SelectMonsterSkill()
{
	NodeName = TEXT("Select Monster Skill");
}

EBTNodeResult::Type UBTTask_SelectMonsterSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	AMonster* Monster = AIController ? Cast<AMonster>(AIController->GetPawn()) : nullptr;
	if (!Monster) return EBTNodeResult::Failed;

	return Monster->SelectWeightedSkill()
		? EBTNodeResult::Succeeded
		: EBTNodeResult::Failed;
}
