#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SelectMonsterSkill.generated.h"

UCLASS()
class UBTTask_SelectMonsterSkill : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SelectMonsterSkill();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
