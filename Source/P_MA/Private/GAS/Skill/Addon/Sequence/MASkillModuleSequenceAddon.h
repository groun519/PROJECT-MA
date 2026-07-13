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

	template<typename ModifierType>
	ModifierType* AddTransientModifier()
	{
		static_assert(TIsDerivedFrom<ModifierType, UMASkillSequenceModifier>::IsDerived,
			"ModifierType must derive from UMASkillSequenceModifier.");
		check(GetOuter() && GetOuter()->HasAnyFlags(RF_Transient));

		ModifierType* Modifier = NewObject<ModifierType>(this, NAME_None, RF_Transient);
		if (Modifier) SequenceModifiers.Add(Modifier);
		return Modifier;
	}

private:
	UPROPERTY(EditDefaultsOnly, Category="Sequence")
	TArray<FMASkillSequence> Sequences;

	UPROPERTY(EditDefaultsOnly, Instanced, Category="Sequence")
	TArray<TObjectPtr<UMASkillSequenceModifier>> SequenceModifiers;

	friend struct FMASkillSequenceAssembler;
};
