#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Sequence/MASkillSequenceModifier.h"
#include "MASkillSequenceModifier_Repeat.generated.h"

/** Repeats one assembled sequence immediately after its original occurrence. */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillSequenceModifier_Repeat : public UMASkillSequenceModifier
{
	GENERATED_BODY()

public:
	virtual void Apply(TArray<FMASkillSequence>& Sequences, UObject& RuntimeOuter) const override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Sequence", meta=(ClampMin="1", UIMin="1"))
	int32 AdditionalCopies = 1;
};
