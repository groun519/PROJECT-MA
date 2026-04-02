#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MASkillAction.generated.h"

struct FGameplayEventData;
struct FSkillRuntimeContext;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillAction : public UObject
{
	GENERATED_BODY()

public:
	virtual void Execute(FSkillRuntimeContext& RuntimeContext, const FGameplayEventData& Payload)
		PURE_VIRTUAL(UMASkillAction::Execute, );
};
