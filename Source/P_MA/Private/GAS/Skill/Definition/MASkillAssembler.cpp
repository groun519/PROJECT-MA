#include "GAS/Skill/Definition/MASkillAssembler.h"

#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/MASkillSystemTypes.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Runtime/MASkillRuntimeRegistry.h"
#include "GAS/Skill/Sequence/MASkillSequenceModifier.h"

struct FMASkillAssemblyState
{
	explicit FMASkillAssemblyState(bool bInPassiveSlot)
		: bPassiveSlot(bInPassiveSlot) {}

	bool TryActivate(UMASkillModuleInstance& ModuleInstance)
	{
		const UMASkillDefinition* Definition = ModuleInstance.GetDefinition();
		if (!Definition) return false;

		if (bPassiveSlot != Definition->GetModuleTags().HasTagExact(PassiveModuleTag))
		{
			ModuleInstance.SetActive(false, PassiveModuleTag);
			return false;
		}

		FGameplayTag BlockingAssemblyTag;
		for (const FGameplayTag& ModuleTag : Definition->GetModuleTags())
		{
			if (!ModuleTag.IsValid() || !ModuleTag.MatchesTag(ExclusiveModuleTag)) continue;

			if (ModuleTag.MatchesTag(UniqueModuleTag))
			{
				if (const TSet<const UMASkillDefinition*>* UsedDefinitions = UsedUniqueDefinitionsByTag.Find(ModuleTag);
					UsedDefinitions && UsedDefinitions->Contains(Definition))
				{
					BlockingAssemblyTag = ModuleTag;
					break;
				}
				continue;
			}

			if (UsedExclusiveModuleTags.Contains(ModuleTag))
			{
				BlockingAssemblyTag = ModuleTag;
				break;
			}
		}

		if (BlockingAssemblyTag.IsValid())
		{
			ModuleInstance.SetActive(false, BlockingAssemblyTag);
			return false;
		}

		for (const FGameplayTag& ModuleTag : Definition->GetModuleTags())
		{
			if (!ModuleTag.IsValid() || !ModuleTag.MatchesTag(ExclusiveModuleTag)) continue;

			if (ModuleTag.MatchesTag(UniqueModuleTag))
			{
				UsedUniqueDefinitionsByTag.FindOrAdd(ModuleTag).Add(Definition);
			}
			else
			{
				UsedExclusiveModuleTags.Add(ModuleTag);
			}
		}

		return true;
	}

	const FGameplayTag ExclusiveModuleTag = FGameplayTag::RequestGameplayTag(TEXT("Module.Exclusive"), false);
	const FGameplayTag UniqueModuleTag = FGameplayTag::RequestGameplayTag(TEXT("Module.Exclusive.Unique"), false);
	const FGameplayTag PassiveModuleTag = FGameplayTag::RequestGameplayTag(TEXT("Module.Passive"));
	const bool bPassiveSlot;
	TSet<FGameplayTag> UsedExclusiveModuleTags;
	TMap<FGameplayTag, TSet<const UMASkillDefinition*>> UsedUniqueDefinitionsByTag;
};

UMASkillModuleInstance* FMASkillAssembler::Assemble(
	UObject* Outer,
	const FGameplayTag& SlotTag,
	const TArray<TObjectPtr<UMASkillModuleInstance>>& OrderedModuleInstances)
{
	if (!Outer) return nullptr;

	UMASkillModuleInstance* AssembledModuleInstance = nullptr;
	UMASkillDefinition* AssembledDefinition = nullptr;
	FMASkillAssemblyState AssemblyState(FMASkillSystemStatics::IsPassiveSkillSlotTag(SlotTag));
	TMap<int32, FText> NameKeywordsByPriority;
	int32 PriorityOneIconCount = 0;

	for (UMASkillModuleInstance* RootModuleInstance : OrderedModuleInstances)
	{
		if (!RootModuleInstance) continue;
		RootModuleInstance->SetActive(true);
	}

	auto AppendDisplayData = [
		&NameKeywordsByPriority,
		&PriorityOneIconCount,
		&AssembledDefinition](const UMASkillDefinition& Definition)
	{
		const FMASkillDefinitionDisplayData& DisplayData = Definition.GetDisplayData();
		const FMASkillDefinitionIconData& IconData = DisplayData.IconData;
		if (IconData.Priority == 1 && IconData.Icon)
		{
			if (PriorityOneIconCount == 0)
			{
				AssembledDefinition->DisplayData.IconData.Icon = IconData.Icon;
			}
			else if (PriorityOneIconCount == 1)
			{
				AssembledDefinition->AssembledSubIcon = IconData.Icon;
			}
			++PriorityOneIconCount;
		}

		const FMASkillDefinitionNameData& NameData = DisplayData.NameData;
		if (NameData.Priority > 0 && !NameData.Keyword.IsEmpty())
		{
			NameKeywordsByPriority.FindOrAdd(NameData.Priority, NameData.Keyword);
		}
	};

	for (UMASkillModuleInstance* RootModuleInstance : OrderedModuleInstances)
	{
		if (!RootModuleInstance || !AssemblyState.TryActivate(*RootModuleInstance)) continue;

		if (!AssembledDefinition)
		{
			AssembledModuleInstance = NewObject<UMASkillModuleInstance>(Outer);
			if (!AssembledModuleInstance) return nullptr;
			AssembledModuleInstance->RuntimeRegistry = NewObject<UMASkillRuntimeRegistry>(AssembledModuleInstance);
			if (!AssembledModuleInstance->RuntimeRegistry) return nullptr;

			AssembledDefinition = NewObject<UMASkillDefinition>(AssembledModuleInstance);
			if (!AssembledDefinition) return nullptr;
			AssembledDefinition->ResetAssemblyData();
			AssembledModuleInstance->SetDefinition(AssembledDefinition);
		}

		const FMASkillScopes TargetScopes(RootModuleInstance, AssembledModuleInstance);
		TArray<FMASkillSequence> ModuleSequences;
		TArray<const UMASkillSequenceModifier*> SequenceModifiers;

		const UMASkillDefinition* Definition = RootModuleInstance->GetDefinition();
		if (!Definition) continue;

		AssembledDefinition->AppendFrom(RootModuleInstance);
		AppendDisplayData(*Definition);
		for (const FMASkillSequence& SourceSequence : Definition->GetBaseSequences())
		{
			FMASkillSequence Sequence = SourceSequence;
			Sequence.TargetScopes = TargetScopes;
			Sequence.Tasks.Reset();
			Sequence.InitialSequenceIndex = 0;
			Sequence.SequenceAdvanceCount = 0;
			ModuleSequences.Add(MoveTemp(Sequence));
		}
		for (const UMASkillSequenceModifier* Modifier : Definition->GetSequenceModifiers())
		{
			if (Modifier) SequenceModifiers.Add(Modifier);
		}

		for (const UMASkillSequenceModifier* Modifier : SequenceModifiers)
		{
			Modifier->Apply(ModuleSequences, *AssembledDefinition);
		}

		AssembledDefinition->AssembledSequences.Append(MoveTemp(ModuleSequences));
	}

	if (AssembledDefinition && !NameKeywordsByPriority.IsEmpty())
	{
		TArray<int32> Priorities;
		NameKeywordsByPriority.GetKeys(Priorities);
		Priorities.Sort();

		FString AssembledName;
		for (const int32 Priority : Priorities)
		{
			const FText* Keyword = NameKeywordsByPriority.Find(Priority);
			if (!Keyword || Keyword->IsEmpty()) continue;

			if (!AssembledName.IsEmpty())
			{
				AssembledName.AppendChar(TEXT(' '));
			}
			AssembledName.Append(Keyword->ToString());
		}

		AssembledDefinition->DisplayData.DisplayName = FText::FromString(AssembledName);
		AssembledDefinition->DisplayData.NameData.Keyword = FText::FromString(AssembledName);
	}

	if (AssembledDefinition)
	{
		AssembledDefinition->FinalizeSequenceAssembly();
	}

	return AssembledModuleInstance;
}
