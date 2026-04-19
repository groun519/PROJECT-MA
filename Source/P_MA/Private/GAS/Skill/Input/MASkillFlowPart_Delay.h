#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Input/MASkillFlowPart_Timed.h"
#include "MASkillFlowPart_Delay.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, HideCategories="Flow")
class P_MA_API UMASkillFlowPart_Delay : public UMASkillFlowPart_Timed
{
	GENERATED_BODY()

public:
	virtual UAnimMontage* ResolveFlowMontage() const override { return nullptr; }
	virtual float GetFlowDuration() const override { return DelayDuration; }

private:
	UPROPERTY(EditDefaultsOnly, Category="Delay", meta=(ClampMin="0.0"))
	float DelayDuration = 0.f;
};
