#include "GAS/Skill/Definition/MASkillAssembler.h"

#include "GAS/Skill/Definition/Assembly/MASkillFeatureAssemblers.h"
#include "GAS/Skill/MASkillSystemTypes.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Runtime/MASkillRuntimeRegistry.h"

struct FMASkillModuleActivationResolver
{
	explicit FMASkillModuleActivationResolver(bool bInPassiveSlot)
		: bPassiveSlot(bInPassiveSlot) {}

	const UMASkillModule* Resolve(UMASkillModuleInstance& ModuleInstance)
	{
		const UMASkillModule* Module = ModuleInstance.GetModule();
		if (!Module) return nullptr;

		if (bPassiveSlot != Module->GetModuleTags().HasTagExact(PassiveModuleTag))
		{
			ModuleInstance.SetActive(false, PassiveModuleTag);
			return nullptr;
		}

		FGameplayTag BlockingAssemblyTag;
		for (const FGameplayTag& ModuleTag : Module->GetModuleTags())
		{
			if (!ModuleTag.IsValid() || !ModuleTag.MatchesTag(ExclusiveModuleTag)) continue;

			if (ModuleTag.MatchesTag(UniqueModuleTag))
			{
				if (const TSet<const UMASkillModule*>* UsedModules = UsedUniqueModulesByTag.Find(ModuleTag);
					UsedModules && UsedModules->Contains(Module))
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

		for (const FGameplayTag& ModuleTag : Module->GetModuleTags())
		{
			if (!ModuleTag.IsValid() || !ModuleTag.MatchesTag(ExclusiveModuleTag)) continue;

			if (ModuleTag.MatchesTag(UniqueModuleTag))
			{
				UsedUniqueModulesByTag.FindOrAdd(ModuleTag).Add(Module);
			}
			else
			{
				UsedExclusiveModuleTags.Add(ModuleTag);
			}
		}

		ModuleInstance.SetActive(true);
		return Module;
	}

	const FGameplayTag ExclusiveModuleTag = FGameplayTag::RequestGameplayTag(TEXT("Module.Exclusive"), false);
	const FGameplayTag UniqueModuleTag = FGameplayTag::RequestGameplayTag(TEXT("Module.Exclusive.Unique"), false);
	const FGameplayTag PassiveModuleTag = FGameplayTag::RequestGameplayTag(TEXT("Module.Passive"));
	const bool bPassiveSlot;
	TSet<FGameplayTag> UsedExclusiveModuleTags;
	TMap<FGameplayTag, TSet<const UMASkillModule*>> UsedUniqueModulesByTag;
};

UMASkillModuleInstance* FMASkillAssembler::Assemble(
	UObject* Outer,
	const FGameplayTag& SlotTag,
	const TArray<TObjectPtr<UMASkillModuleInstance>>& OrderedModuleInstances)
{
	check(Outer);

	UMASkillModuleInstance* AssembledModuleInstance = nullptr;
	UMASkillModule* AssembledModule = nullptr;
	FMASkillModuleActivationResolver ActivationResolver(FMASkillSystemStatics::IsPassiveSkillSlotTag(SlotTag));
	TMap<int32, FText> NameKeywordsByPriority;
	int32 PriorityOneIconCount = 0;

	auto AppendDisplayData = [
		&NameKeywordsByPriority,
		&PriorityOneIconCount,
		&AssembledModule](const UMASkillModule& Module)
	{
		const FMASkillDefinitionDisplayData& DisplayData = Module.GetDisplayData();
		const FMASkillDefinitionIconData& IconData = DisplayData.IconData;
		if (IconData.Priority == 1 && IconData.Icon)
		{
			if (PriorityOneIconCount == 0)
			{
				AssembledModule->ModuleData.DisplayData.IconData.Icon = IconData.Icon;
			}
			else if (PriorityOneIconCount == 1)
			{
				AssembledModule->ModuleData.AssembledSubIcon = IconData.Icon;
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

		const UMASkillModule* Module = ActivationResolver.Resolve(*RootModuleInstance);
		if (!Module) continue;

		if (!AssembledModule)
		{
			AssembledModuleInstance = NewObject<UMASkillModuleInstance>(Outer);
			check(AssembledModuleInstance);
			AssembledModuleInstance->RuntimeRegistry = NewObject<UMASkillRuntimeRegistry>(AssembledModuleInstance);
			check(AssembledModuleInstance->RuntimeRegistry);

			AssembledModule = NewObject<UMASkillModule>(AssembledModuleInstance);
			check(AssembledModule);
			AssembledModule->ResetAssemblyData();
			AssembledModuleInstance->SetModule(AssembledModule);
		}

		const FMASkillScopes TargetScopes(RootModuleInstance, AssembledModuleInstance);

		AssembledModule->ModuleData.ModuleVisualTags.AppendTags(Module->GetModuleData().ModuleVisualTags);
		AppendDisplayData(*Module);
		FMASkillCooldownAssembler::AppendFrom(*AssembledModule, *Module);
		FMASkillPayloadAssembler::AppendFrom(*AssembledModule, *Module);
		FMASkillEventSourceAssembler::AppendFrom(*AssembledModule, *Module);
		FMASkillEventBindingAssembler::AppendFrom(*AssembledModule, *Module, *RootModuleInstance, *AssembledModuleInstance);
		FMASkillSequenceAssembler::AppendFrom(*AssembledModule, *Module, TargetScopes);
	}

	if (AssembledModule && !NameKeywordsByPriority.IsEmpty())
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

		AssembledModule->ModuleData.DisplayData.DisplayName = FText::FromString(AssembledName);
		AssembledModule->ModuleData.DisplayData.NameData.Keyword = FText::FromString(AssembledName);
	}

	if (AssembledModule)
	{
		FMASkillSequenceAssembler::Finalize(*AssembledModule);
	}

	return AssembledModuleInstance;
}
