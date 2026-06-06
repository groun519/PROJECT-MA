#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTTask_MoveToSelectedSkillRange.generated.h"

UCLASS()
class UBTTask_MoveToSelectedSkillRange : public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	UBTTask_MoveToSelectedSkillRange();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
