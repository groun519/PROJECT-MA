#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Addon/MASkillModuleAddon.h"
#include "GAS/Skill/Sequence/MASkillSequenceModifier.h"
#include "GAS/Skill/Sequence/MASkillSequenceTypes.h"
#include "MASkillModuleSequenceAddon.generated.h"

/** Contributes animation sequences and their assembly modifiers to a skill. */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillModuleSequenceAddon : public UMASkillModuleAddon
{
	GENERATED_BODY()

public:
	const TArray<FMASkillSequence>& GetSequences() const { return Sequences; }
	const TArray<TObjectPtr<UMASkillSequenceModifier>>& GetSequenceModifiers() const { return SequenceModifiers; }

#if WITH_EDITOR
	void AddGeneratedModifier(UMASkillSequenceModifier& Modifier)
	{
		SequenceModifiers.Add(&Modifier);
	}
#endif

private:
	UPROPERTY(EditDefaultsOnly, Category="Sequence")
	TArray<FMASkillSequence> Sequences;

	UPROPERTY(EditDefaultsOnly, Instanced, Category="Sequence")
	TArray<TObjectPtr<UMASkillSequenceModifier>> SequenceModifiers;

	friend struct FMASkillSequenceAssembler;
};
