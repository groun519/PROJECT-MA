#include "GAS/Skill/Definition/MASkillDefinition.h"

#include "GAS/Skill/Addon/Stack/MASkillModuleStackAddon.h"
#include "GAS/Skill/Event/Source/MASkillEventSource.h"
#include "GAS/Skill/Module/MASkillModuleAddonRuntimeData.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"

static void MoveVisualTags(
	FGameplayTagContainer& SourceTags,
	FGameplayTagContainer& TargetTags)
{
	static const FGameplayTag ModuleVisualTag = FGameplayTag::RequestGameplayTag(TEXT("Module.Visual"));

	FGameplayTagContainer VisualTags;
	for (const FGameplayTag& SourceTag : SourceTags)
	{
		if (SourceTag.IsValid() && SourceTag.MatchesTag(ModuleVisualTag))
		{
			VisualTags.AddTag(SourceTag);
		}
	}

	if (VisualTags.IsEmpty()) return;

	TargetTags.AppendTags(VisualTags);
	SourceTags.RemoveTags(VisualTags);
}

FMASkillIconData UMASkillDefinition::ResolveIconData(const UMAModuleQualityData* ModuleQualityData) const
{
	FMASkillIconData IconData;
	IconData.Icon = DisplayData.IconData.Icon;
	if (!ModuleQualityData) return IconData;

	if (const FMAModuleTypeData* VisualData = ModuleQualityData->FindVisualData(ModuleVisualTags))
	{
		IconData.IconColor = VisualData->IconColor;
		IconData.InnerColor = VisualData->InnerColor;
	}
	return IconData;
}

FLinearColor UMASkillDefinition::ResolveFrameColor(const UMAModuleQualityData* ModuleQualityData) const
{
	const FMAModuleRarityData* RarityData = ModuleQualityData
		? ModuleQualityData->FindRarityData(ModuleQuality.Rarity)
		: nullptr;
	return RarityData ? RarityData->Color : FLinearColor::White;
}

FGameplayTagContainer UMASkillDefinition::GetTooltipTags() const
{
	return ModuleTags;
}

FGameplayTag UMASkillDefinition::GetVisualElementTag() const
{
	static const FGameplayTag ElementalVisualTag = FGameplayTag::RequestGameplayTag(TEXT("Module.Visual.Elemental"));
	for (const FGameplayTag& VisualTag : ModuleVisualTags)
	{
		if (VisualTag != ElementalVisualTag && VisualTag.MatchesTag(ElementalVisualTag))
		{
			return VisualTag;
		}
	}

	return FGameplayTag();
}

const UMASkillModuleStackAddon* UMASkillDefinition::GetStackAddon() const
{
	return FindAddon<UMASkillModuleStackAddon>();
}

void UMASkillDefinition::PostLoad()
{
	Super::PostLoad();

	if (ExclusiveAssemblyTag_DEPRECATED.IsValid())
	{
		ModuleTags.AddTag(ExclusiveAssemblyTag_DEPRECATED);
	}
	if (UniqueModuleEffectTag_DEPRECATED.IsValid())
	{
		ModuleTags.AddTag(UniqueModuleEffectTag_DEPRECATED);
	}
	MoveVisualTags(ModuleTags, ModuleVisualTags);

	for (FMASkillEventBinding& EventBinding : EventBindings)
	{
		if (EventBinding.bUseLocalBinding)
		{
			EventBinding.BindingScope = EMASkillEventBindingScope::Module;
		}
		EventBinding.bUseLocalBinding = false;
	}
}

bool UMASkillDefinition::HasEventSource(FGameplayTag EventTag) const
{
	if (!EventTag.IsValid()) return false;

	for (const UMASkillEventSource* EventSource : EventSources)
	{
		if (EventSource && EventSource->GetEmittedTag() == EventTag) return true;
	}

	return false;
}

void UMASkillDefinition::InitializeAddonRuntimeData(
	FMASkillModuleAddonRuntimeData& RuntimeData) const
{
	ForEachUniqueAddon([&](const UMASkillModuleAddon& Addon)
	{
		Addon.InitializeRuntimeData(RuntimeData);
	});
}

void UMASkillDefinition::ApplyAddonPayloadMirrors(
	const FMASkillModuleAddonRuntimeData& RuntimeData,
	FMASkillPayloadStore& PayloadStore) const
{
	ForEachUniqueAddon([&](const UMASkillModuleAddon& Addon)
	{
		Addon.ApplyPayloadMirror(RuntimeData, PayloadStore);
	});
}

bool UMASkillDefinition::TryResolveSocketText(
	const FMASkillModuleAddonRuntimeData& RuntimeData,
	FText& OutText) const
{
	bool bResolved = false;
	ForEachUniqueAddon([&](const UMASkillModuleAddon& Addon)
	{
		if (!bResolved)
		{
			bResolved = Addon.TryResolveSocketText(RuntimeData, OutText);
		}
	});
	return bResolved;
}

void UMASkillDefinition::ForEachUniqueAddon(
	TFunctionRef<void(const UMASkillModuleAddon&)> Func) const
{
	TSet<const UClass*> SeenAddonClasses;
	for (const TObjectPtr<UMASkillModuleAddon>& Addon : Addons)
	{
		if (!Addon) continue;

		const UClass* AddonClass = Addon->GetClass();
		if (SeenAddonClasses.Contains(AddonClass))
		{
			ensureMsgf(false,
				TEXT("Duplicate module addon '%s' in '%s'."),
				*GetNameSafe(AddonClass),
				*GetName());
			continue;
		}

		SeenAddonClasses.Add(AddonClass);
		Func(*Addon);
	}
}

void UMASkillDefinition::ResetAssemblyData()
{
	DisplayData = FMASkillDefinitionDisplayData();
	AssembledSubIcon = nullptr;
	ModuleTags.Reset();
	ModuleVisualTags.Reset();
	ExclusiveAssemblyTag_DEPRECATED = FGameplayTag();
	UniqueModuleEffectTag_DEPRECATED = FGameplayTag();
	CooldownSeconds = 0.f;
	ModuleCooldown = FMASkillModuleCooldownConfig();
	BaseSequences.Reset();
	SequenceModifiers.Reset();
	AssembledSequences.Reset();
	Addons.Reset();
	EventSources.Reset();
	EventBindings.Reset();
	Payloads.Reset();
}


