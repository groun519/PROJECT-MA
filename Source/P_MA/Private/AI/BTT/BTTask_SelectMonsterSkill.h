#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SelectMonsterSkill.generated.h"

class AMonster;
class UStateTree;
struct FStateTreeDataView;
struct FStateTreeExecutionContext;
struct FStateTreeExternalDataDesc;

UCLASS()
class UBTTask_SelectMonsterSkill : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SelectMonsterSkill();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	bool BuildPatternPlan(UBehaviorTreeComponent& OwnerComp, AMonster& Monster);
	bool CollectExternalData(
		const FStateTreeExecutionContext& Context,
		const UStateTree* StateTree,
		TArrayView<const FStateTreeExternalDataDesc> ExternalDataDescs,
		TArrayView<FStateTreeDataView> OutDataViews);
};
