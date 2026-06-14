#include "GAS/Skill/Definition/MASkillAssembler.h"

#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Runtime/MASkillRuntimeRegistry.h"

UMASkillModuleInstance* FMASkillAssembler::Assemble(UObject* Outer, const TArray<TObjectPtr<UMASkillModuleInstance>>& OrderedModuleInstances)
{
	if (!Outer) return nullptr;

	UMASkillModuleInstance* AssembledModuleInstance = nullptr;
	UMASkillDefinition* AssembledDefinition = nullptr;
	TMap<int32, FText> NameKeywordsByPriority;
	TSet<FGameplayTag> UsedExclusiveAssemblyTags;
	TMap<FGameplayTag, TSet<const UMASkillDefinition*>> UsedUniqueDefinitionsByTag;
	const FGameplayTag UniqueAssemblyTag = FGameplayTag::RequestGameplayTag(TEXT("Module.Assembly.Exclusive.Unique"), false);
	int32 PriorityOneIconCount = 0;
	bool bHasIconColors = false;

	for (UMASkillModuleInstance* ModuleInstance : OrderedModuleInstances)
	{
		if (ModuleInstance)
		{
			ModuleInstance->SetActivationState(EMASkillModuleActivationState::Active);
		}
	}

	for (UMASkillModuleInstance* ModuleInstance : OrderedModuleInstances)
	{
		UMASkillDefinition* Definition = ModuleInstance ? ModuleInstance->GetDefinition() : nullptr;
		if (!Definition) continue;

		FGameplayTag BlockingAssemblyTag;
		for (const FGameplayTag& ExclusiveAssemblyTag : Definition->GetExclusiveAssemblyTags())
		{
			if (!ExclusiveAssemblyTag.IsValid()) continue;

			if (ExclusiveAssemblyTag.MatchesTag(UniqueAssemblyTag))
			{
				if (const TSet<const UMASkillDefinition*>* UsedDefinitions = UsedUniqueDefinitionsByTag.Find(ExclusiveAssemblyTag);
					UsedDefinitions && UsedDefinitions->Contains(Definition))
				{
					BlockingAssemblyTag = ExclusiveAssemblyTag;
					break;
				}
				continue;
			}

			if (UsedExclusiveAssemblyTags.Contains(ExclusiveAssemblyTag))
			{
				BlockingAssemblyTag = ExclusiveAssemblyTag;
				break;
			}
		}

		if (BlockingAssemblyTag.IsValid())
		{
			ModuleInstance->SetActivationState(EMASkillModuleActivationState::Inactive, BlockingAssemblyTag);
			continue;
		}

		for (const FGameplayTag& ExclusiveAssemblyTag : Definition->GetExclusiveAssemblyTags())
		{
			if (!ExclusiveAssemblyTag.IsValid()) continue;

			if (ExclusiveAssemblyTag.MatchesTag(UniqueAssemblyTag))
			{
				UsedUniqueDefinitionsByTag.FindOrAdd(ExclusiveAssemblyTag).Add(Definition);
			}
			else
			{
				UsedExclusiveAssemblyTags.Add(ExclusiveAssemblyTag);
			}
		}

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

		AssembledDefinition->AppendFrom(ModuleInstance);

		const FMASkillDefinitionDisplayData& DisplayData = Definition->DisplayData;
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
		if (!bHasIconColors && IconData.Priority == 2)
		{
			AssembledDefinition->DisplayData.IconData.IconColor = IconData.IconColor;
			AssembledDefinition->DisplayData.IconData.InnerColor = IconData.InnerColor;
			bHasIconColors = true;
		}

		const FMASkillDefinitionNameData& NameData = DisplayData.NameData;
		if (NameData.Priority > 0 && !NameData.Keyword.IsEmpty())
		{
			NameKeywordsByPriority.FindOrAdd(NameData.Priority, NameData.Keyword);
		}
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
		AssembledDefinition->FinalizeStepAssembly();
	}

	return AssembledModuleInstance;
}
