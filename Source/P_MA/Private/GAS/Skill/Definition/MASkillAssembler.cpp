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
	TSet<FGameplayTag> UsedExclusiveModuleTags;
	TMap<FGameplayTag, TSet<const UMASkillDefinition*>> UsedUniqueDefinitionsByTag;
	const FGameplayTag ExclusiveModuleTag = FGameplayTag::RequestGameplayTag(TEXT("Module.Exclusive"), false);
	const FGameplayTag UniqueModuleTag = FGameplayTag::RequestGameplayTag(TEXT("Module.Exclusive.Unique"), false);
	int32 PriorityOneIconCount = 0;
	bool bHasIconColors = false;

	for (UMASkillModuleInstance* ModuleInstance : OrderedModuleInstances)
	{
		if (ModuleInstance)
		{
			ModuleInstance->SetActive(true);
		}
	}

	for (UMASkillModuleInstance* ModuleInstance : OrderedModuleInstances)
	{
		UMASkillDefinition* Definition = ModuleInstance ? ModuleInstance->GetDefinition() : nullptr;
		if (!Definition) continue;

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
			ModuleInstance->SetActive(false, BlockingAssemblyTag);
			continue;
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
