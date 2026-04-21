#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Step/MASkillStep_Timed.h"
#include "MASkillStep_Delay.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, HideCategories="Step")
class P_MA_API UMASkillStep_Delay : public UMASkillStep_Timed
{
	GENERATED_BODY()

public:
	virtual UAnimMontage* ResolveStepMontage() const override { return nullptr; }
	virtual float GetStepDuration() const override { return DelayDuration; }

private:
	UPROPERTY(EditDefaultsOnly, Category="Delay", meta=(ClampMin="0.0"))
	float DelayDuration = 0.f;
};
