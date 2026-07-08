#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Sequence/MASkillSequenceTypes.h"
#include "UObject/Object.h"
#include "MASkillSequenceModifier.generated.h"

class UMASkillSequenceTask;
struct FMASkillSequenceTaskConfig;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillSequenceModifier : public UObject
{
	GENERATED_BODY()

public:
	virtual void Apply(
		TArray<FMASkillSequence>& Sequences,
		UObject& RuntimeOuter) const PURE_VIRTUAL(UMASkillSequenceModifier::Apply, );

protected:
	// If progress options grow beyond label/visibility, move them into a shared options struct
	// instead of duplicating progress fields across modifier variants.
	UMASkillSequenceTask* AddTask(
		TArray<FMASkillSequence>& Sequences,
		UObject& RuntimeOuter,
		const FMASkillSequenceTaskConfig& Config) const;

	UPROPERTY(EditDefaultsOnly, Category="Sequence", meta=(ClampMin="0"))
	int32 TargetSequenceIndex = 0;
};
