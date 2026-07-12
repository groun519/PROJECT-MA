#include "GAS/Skill/Definition/MASkillAssembler.h"

#include "GAS/Skill/Definition/Assembly/MASkillFeatureAssemblers.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/MASkillSystemTypes.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Runtime/MASkillRuntimeRegistry.h"

struct FMASkillModuleActivationResolver
{
	explicit FMASkillModuleActivationResolver(bool bInPassiveSlot)
		: bPassiveSlot(bInPassiveSlot) {}

	const UMASkillDefinition* Resolve(UMASkillModuleInstance& ModuleInstance)
	{
		const UMASkillDefinition* Definition = ModuleInstance.GetDefinition();
		if (!Definition) return nullptr;

		if (bPassiveSlot != Definition->GetModuleTags().HasTagExact(PassiveModuleTag))
		{
			ModuleInstance.SetActive(false, PassiveModuleTag);
			return nullptr;
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
			return nullptr;
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

		ModuleInstance.SetActive(true);
		return Definition;
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
	check(Outer);

	UMASkillModuleInstance* AssembledModuleInstance = nullptr;
	UMASkillDefinition* AssembledDefinition = nullptr;
	FMASkillModuleActivationResolver ActivationResolver(FMASkillSystemStatics::IsPassiveSkillSlotTag(SlotTag));
	TMap<int32, FText> NameKeywordsByPriority;
	int32 PriorityOneIconCount = 0;

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
		if (!RootModuleInstance) continue;

		const UMASkillDefinition* Definition = ActivationResolver.Resolve(*RootModuleInstance);
		if (!Definition) continue;

		if (!AssembledDefinition)
		{
			AssembledModuleInstance = NewObject<UMASkillModuleInstance>(Outer);
			check(AssembledModuleInstance);
			AssembledModuleInstance->RuntimeRegistry = NewObject<UMASkillRuntimeRegistry>(AssembledModuleInstance);
			check(AssembledModuleInstance->RuntimeRegistry);

			AssembledDefinition = NewObject<UMASkillDefinition>(AssembledModuleInstance);
			check(AssembledDefinition);
			AssembledDefinition->ResetAssemblyData();
			AssembledModuleInstance->SetDefinition(AssembledDefinition);
		}

		const FMASkillScopes TargetScopes(RootModuleInstance, AssembledModuleInstance);

		AssembledDefinition->ModuleVisualTags.AppendTags(Definition->ModuleVisualTags);
		AppendDisplayData(*Definition);
		FMASkillCooldownAssembler::AppendFrom(*AssembledDefinition, *Definition);
		FMASkillPayloadAssembler::AppendFrom(*AssembledDefinition, *Definition);
		FMASkillEventAssembler::AppendFrom(*AssembledDefinition, *Definition, *RootModuleInstance, *AssembledModuleInstance);
		FMASkillSequenceAssembler::AppendFrom(*AssembledDefinition, *Definition, TargetScopes);
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
		FMASkillSequenceAssembler::Finalize(*AssembledDefinition);
	}

	return AssembledModuleInstance;
}
