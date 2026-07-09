#include "GAS/Skill/Definition/MASkillDefinition.h"

#include "GAS/Skill/Event/Source/MASkillEventSource.h"

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

void UMASkillDefinition::ResetAssemblyData()
{
	DisplayData = FMASkillDefinitionDisplayData();
	AssembledSubIcon = nullptr;
	ModuleTags.Reset();
	ModuleVisualTags.Reset();
	bStackEnabled = false;
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


