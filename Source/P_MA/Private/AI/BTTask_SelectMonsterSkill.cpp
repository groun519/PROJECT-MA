#include "AI/BTTask_SelectMonsterSkill.h"

#include "AIController.h"
#include "AI/Golem/Monster.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Components/StateTreeComponentSchema.h"
#include "StateTree.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeInstanceData.h"

UBTTask_SelectMonsterSkill::UBTTask_SelectMonsterSkill()
{
	NodeName = TEXT("Select Monster Skill");
}

EBTNodeResult::Type UBTTask_SelectMonsterSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	AMonster* Monster = AIController ? Cast<AMonster>(AIController->GetPawn()) : nullptr;
	if (!Monster) return EBTNodeResult::Failed;

	if (Monster->GetPatternStateTree())
	{
		if ((Monster->HasPendingPatternPlan() || BuildPatternPlan(OwnerComp, *Monster))
			&& Monster->SelectNextPatternPlanFragment())
		{
			return EBTNodeResult::Succeeded;
		}
	}

	return Monster->SelectWeightedSkill()
		? EBTNodeResult::Succeeded
		: EBTNodeResult::Failed;
}

bool UBTTask_SelectMonsterSkill::BuildPatternPlan(UBehaviorTreeComponent& OwnerComp, AMonster& Monster)
{
	const UStateTree* PatternStateTree = Monster.GetPatternStateTree();
	if (!PatternStateTree) return false;

	FStateTreeInstanceData InstanceData;
	FStateTreeExecutionContext Context(*OwnerComp.GetOwner(), *PatternStateTree, InstanceData);
	Context.SetCollectExternalDataCallback(FOnCollectStateTreeExternalData::CreateUObject(
		this,
		&UBTTask_SelectMonsterSkill::CollectExternalData));
	Context.SetContextDataByName(TEXT("AIController"), FStateTreeDataView(OwnerComp.GetAIOwner()));
	if (!UStateTreeComponentSchema::SetContextRequirements(OwnerComp, Context)) return false;

	TArray<FName> SelectedStateNames;
	auto CaptureActiveStateNames = [&Context, &SelectedStateNames]()
	{
		const TArray<FName> ActiveStateNames = Context.GetActiveStateNames();
		if (!ActiveStateNames.IsEmpty())
		{
			SelectedStateNames = ActiveStateNames;
		}
	};

	EStateTreeRunStatus RunStatus = Context.Start();
	CaptureActiveStateNames();

	for (int32 Iteration = 0; RunStatus == EStateTreeRunStatus::Running && Iteration < 8; ++Iteration)
	{
		RunStatus = Context.Tick(0.f);
		CaptureActiveStateNames();
	}

	if (RunStatus == EStateTreeRunStatus::Running)
	{
		Context.Stop();
	}

	if (!Monster.SetPatternPlanFromStateNames(SelectedStateNames))
	{
		return false;
	}

	return true;
}

bool UBTTask_SelectMonsterSkill::CollectExternalData(
	const FStateTreeExecutionContext& Context,
	const UStateTree* StateTree,
	TArrayView<const FStateTreeExternalDataDesc> ExternalDataDescs,
	TArrayView<FStateTreeDataView> OutDataViews)
{
	return UStateTreeComponentSchema::CollectExternalData(Context, StateTree, ExternalDataDescs, OutDataViews);
}
