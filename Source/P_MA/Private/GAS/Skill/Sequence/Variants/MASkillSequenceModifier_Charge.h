#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Sequence/MASkillSequenceModifier.h"
#include "MASkillSequenceModifier_Charge.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillSequenceModifier_Charge : public UMASkillSequenceModifier
{
	GENERATED_BODY()

public:
	UMASkillSequenceModifier_Charge();
	virtual void Apply(TArray<FMASkillSequence>& Sequences, UObject& RuntimeOuter) const override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Time", meta=(ClampMin="0.0", UIMin="0.0"))
	float TimeLimitSeconds = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Progress")
	bool bShowProgress = true;

	UPROPERTY(EditDefaultsOnly, Category="Progress", meta=(EditCondition="bShowProgress"))
	FText ProgressLabel;
};
