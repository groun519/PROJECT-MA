#include "AI/BTT/BTTask_MoveToSelectedSkillRange.h"

#include "AIController.h"
#include "AI/Monster/Monster.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_MoveToSelectedSkillRange::UBTTask_MoveToSelectedSkillRange()
{
	NodeName = TEXT("Move To Selected Skill Range");
	bCreateNodeInstance = true;
	AcceptableRadius = 300.f;
}

EBTNodeResult::Type UBTTask_MoveToSelectedSkillRange::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const AMonster* Monster = AIController ? Cast<AMonster>(AIController->GetPawn()) : nullptr;
	if (!Monster) return EBTNodeResult::Failed;

	AcceptableRadius = Monster->HasSelectedSkill()
		? Monster->GetSelectedSkillUseDistance()
		: 300.f;

	return Super::ExecuteTask(OwnerComp, NodeMemory);
}
