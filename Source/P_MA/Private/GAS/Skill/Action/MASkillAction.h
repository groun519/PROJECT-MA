#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MASkillAction.generated.h"

struct FGameplayEventData;
struct FMASkillPayloadStore;
struct FSkillRuntimeContext;
class UMASkillAbility;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillAction : public UObject
{
	GENERATED_BODY()

public:
	virtual void Execute(UMASkillAbility& OwnerAbility, FSkillRuntimeContext& RuntimeContext, FMASkillPayloadStore& PayloadStore, const FGameplayEventData& Payload)
		PURE_VIRTUAL(UMASkillAction::Execute, );
};
