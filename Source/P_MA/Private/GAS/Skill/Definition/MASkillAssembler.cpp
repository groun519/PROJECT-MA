#include "GAS/Skill/Definition/MASkillAssembler.h"

#include "GAS/Skill/Definition/Assembly/MASkillAddonAssembler.h"
#include "GAS/Skill/MASkillSystemTypes.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Runtime/MASkillRuntimeRegistry.h"

struct FMASkillModuleActivationResolver
{
	explicit FMASkillModuleActivationResolver(bool bInPassiveSlot)
		: bPassiveSlot(bInPassiveSlot) {}

	bool Activate(UMASkillModuleInstance& ModuleInstance)
	{
		const UMASkillModule* RootModule = ModuleInstance.GetRootModule();
		if (!RootModule) return false;

		if (bPassiveSlot != RootModule->GetModuleTags().HasTagExact(PassiveModuleTag))
		{
			ModuleInstance.SetActive(false, PassiveModuleTag);
			return false;
		}

		FGameplayTag BlockingAssemblyTag;
		for (const FGameplayTag& ModuleTag : RootModule->GetModuleTags())
		{
			if (!ModuleTag.IsValid() || !ModuleTag.MatchesTag(ExclusiveModuleTag)) continue;

			if (ModuleTag.MatchesTag(UniqueModuleTag))
			{
				if (const TSet<const UMASkillModule*>* UsedModules = UsedUniqueModulesByTag.Find(ModuleTag);
					UsedModules && UsedModules->Contains(RootModule))
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

		for (const FGameplayTag& ModuleTag : RootModule->GetModuleTags())
		{
			if (!ModuleTag.IsValid() || !ModuleTag.MatchesTag(ExclusiveModuleTag)) continue;

			if (ModuleTag.MatchesTag(UniqueModuleTag))
			{
				UsedUniqueModulesByTag.FindOrAdd(ModuleTag).Add(RootModule);
			}
			else
			{
				UsedExclusiveModuleTags.Add(ModuleTag);
			}
		}

		ModuleInstance.SetActive(true);
		return true;
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
	FMASkillModuleData* AssembledData = nullptr;
	FMASkillModuleActivationResolver ActivationResolver(FMASkillSystemStatics::IsPassiveSkillSlotTag(SlotTag));
	TMap<int32, FText> NameKeywordsByPriority;
	int32 PriorityOneIconCount = 0;

	auto AppendDisplayData = [
		&NameKeywordsByPriority,
		&PriorityOneIconCount,
		&AssembledData](const UMASkillModule& Module)
	{
		const FMASkillDefinitionDisplayData& DisplayData = Module.GetDisplayData();
		const FMASkillDefinitionIconData& IconData = DisplayData.IconData;
		if (IconData.Priority == 1 && IconData.Icon)
		{
			if (PriorityOneIconCount == 0)
			{
				AssembledData->DisplayData.IconData.Icon = IconData.Icon;
			}
			else if (PriorityOneIconCount == 1)
			{
				AssembledData->AssembledSubIcon = IconData.Icon;
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

		if (!ActivationResolver.Activate(*RootModuleInstance)) continue;

		const UMASkillModule* ComposedModule = RootModuleInstance->ResolveComposedModule();
		if (!ComposedModule) continue;

		if (!AssembledModule)
		{
			AssembledModuleInstance = NewObject<UMASkillModuleInstance>(Outer);
			check(AssembledModuleInstance);
			AssembledModuleInstance->RuntimeRegistry = NewObject<UMASkillRuntimeRegistry>(AssembledModuleInstance);
			check(AssembledModuleInstance->RuntimeRegistry);

			AssembledModule = NewObject<UMASkillModule>(AssembledModuleInstance);
			check(AssembledModule);
			AssembledData = &AssembledModule->BeginAssembly();
		}

		const FMASkillScopes TargetScopes(RootModuleInstance, AssembledModuleInstance);
		AppendDisplayData(*ComposedModule);
		AssembledData->ModuleVisualTags.AppendTags(
			ComposedModule->GetModuleData().ModuleVisualTags);
		AssembledData->Payloads.Append(ComposedModule->GetModuleData().Payloads);
		FMASkillAddonAssembler::AppendFrom(
			*AssembledModule,
			*AssembledData,
			*ComposedModule,
			EMASkillAddonAssemblyStage::SkillAssembly,
			TargetScopes);
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

		AssembledData->DisplayData.DisplayName = FText::FromString(AssembledName);
		AssembledData->DisplayData.NameData.Keyword = FText::FromString(AssembledName);
	}

	if (AssembledModule)
	{
		FMASkillAddonAssembler::Finalize(
			*AssembledData,
			EMASkillAddonAssemblyStage::SkillAssembly);
		verify(AssembledModuleInstance->SetRootModule(AssembledModule));
	}

	return AssembledModuleInstance;
}
