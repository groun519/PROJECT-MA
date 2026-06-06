#include "AI/BTTask_MoveToSelectedSkillRange.h"

#include "AIController.h"
#include "AI/Golem/Monster.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_MoveToSelectedSkillRange::UBTTask_MoveToSelectedSkillRange()
{
	NodeName = TEXT("Move To Selected Skill Range");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_MoveToSelectedSkillRange::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const AMonster* Monster = AIController ? Cast<AMonster>(AIController->GetPawn()) : nullptr;
	if (!Monster || !Monster->HasSelectedSkill()) return EBTNodeResult::Failed;

	AcceptableRadius = Monster->GetSelectedSkillUseDistance();
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}
