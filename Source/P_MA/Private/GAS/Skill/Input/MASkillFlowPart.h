#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MASkillFlowPart.generated.h"

class UMASkillAbility;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillFlowPart : public UObject
{
	GENERATED_BODY()

public:
	virtual void StartFlow(UMASkillAbility* SkillAbility) { OwnerSkillAbility = SkillAbility; }
	virtual void StopFlow() { OwnerSkillAbility = nullptr; }

protected:
	UMASkillAbility* GetOwnerSkillAbility() const { return OwnerSkillAbility; }

	UPROPERTY(Transient)
	TObjectPtr<UMASkillAbility> OwnerSkillAbility;
};
