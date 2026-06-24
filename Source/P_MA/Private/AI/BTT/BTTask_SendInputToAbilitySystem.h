#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SendInputToAbilitySystem.generated.h"

class UBehaviorTreeComponent;
class UMASkillAbility;

UCLASS()
class UBTTask_SendInputToAbilitySystem : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_SendInputToAbilitySystem();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	void HandleSkillDeactivated();
	void UnbindSkillDeactivated();

	TWeakObjectPtr<UBehaviorTreeComponent> OwnerBehaviorTree;
	TWeakObjectPtr<UMASkillAbility> ActiveSkillAbility;
	FDelegateHandle SkillDeactivatedHandle;
};
