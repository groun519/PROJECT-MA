#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Sequence/MASkillSequenceModifier.h"
#include "MASkillSequenceModifier_Cast.generated.h"

class UAnimMontage;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillSequenceModifier_Cast : public UMASkillSequenceModifier
{
	GENERATED_BODY()

public:
	UMASkillSequenceModifier_Cast();
	virtual void Apply(TArray<FMASkillSequence>& Sequences, UObject& RuntimeOuter) const override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Time", meta=(ClampMin="0.0", UIMin="0.0"))
	float TimeLimitSeconds = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Progress")
	bool bShowProgress = true;

	UPROPERTY(EditDefaultsOnly, Category="Progress", meta=(EditCondition="bShowProgress"))
	FText ProgressLabel;

	UPROPERTY(EditDefaultsOnly, Category="Cast")
	TObjectPtr<UAnimMontage> CustomMontage;
};
