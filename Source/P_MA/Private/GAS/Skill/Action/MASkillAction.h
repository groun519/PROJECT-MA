#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/MASkillEventTypes.h"
#include "UObject/Object.h"
#include "MASkillAction.generated.h"

class UMASkillAbility;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillAction : public UObject
{
	GENERATED_BODY()

public:
	virtual void Execute(
		UMASkillAbility& OwnerAbility,
		const FMASkillEvent& Event,
		const FMASkillScopes& Scopes)
		PURE_VIRTUAL(UMASkillAction::Execute, );
};
