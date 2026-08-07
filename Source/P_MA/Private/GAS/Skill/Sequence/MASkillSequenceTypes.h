#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/MASkillEventTypes.h"
#include "MASkillSequenceTypes.generated.h"

class UAnimMontage;
class UMASkillSequenceTask;

USTRUCT(BlueprintType)
struct P_MA_API FMASkillSequence
{
	GENERATED_BODY()

	bool UsesSequenceSections() const { return !SequenceSectionNameBase.IsNone(); }
	FString GetSequenceKey() const;

	UPROPERTY(EditDefaultsOnly, Category="Montage")
	TObjectPtr<UAnimMontage> Montage = nullptr;

	// Sequences with the same montage and section base share one section sequence.
	UPROPERTY(EditDefaultsOnly, Category="Montage")
	FName SequenceSectionNameBase = NAME_None;

	UPROPERTY(EditDefaultsOnly, Category="Montage", meta=(ClampMin="0"))
	int32 MaxSectionCount = 0;

	// Normalizes the selected section to one second before applying attack-speed multipliers.
	UPROPERTY(EditDefaultsOnly, Category="Montage")
	bool bScaleWithAttackSpeed = false;

	UPROPERTY(Transient)
	FMASkillScopes TargetScopes;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMASkillSequenceTask>> Tasks;

	UPROPERTY(Transient)
	int32 InitialSequenceIndex = 0;

	UPROPERTY(Transient)
	int32 SequenceAdvanceCount = 0;
};
