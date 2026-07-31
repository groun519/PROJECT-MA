#include "GAS/Skill/Module/MASkillModule.h"

#include "GAS/Skill/Addon/Cooldown/MASkillCooldownAddon.h"
#include "GAS/Skill/Addon/Stack/MASkillModuleStackAddon.h"
#include "GAS/Skill/Module/MASkillModuleAddonRuntimeData.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "Engine/AssetManager.h"

const FPrimaryAssetType UMASkillModule::PrimaryAssetType(TEXT("SkillModule"));

FPrimaryAssetId UMASkillModule::MakePrimaryAssetId(const int32 ModuleId)
{
	return ModuleId > 0
		? FPrimaryAssetId(PrimaryAssetType, FName(*LexToString(ModuleId)))
		: FPrimaryAssetId();
}

UMASkillModule* UMASkillModule::LoadById(const int32 ModuleId)
{
	const FSoftObjectPath ModulePath = UAssetManager::Get().GetPrimaryAssetPath(
		MakePrimaryAssetId(ModuleId));
	return Cast<UMASkillModule>(ModulePath.TryLoad());
}

FPrimaryAssetId UMASkillModule::GetPrimaryAssetId() const
{
	return MakePrimaryAssetId(ModuleId);
}

FMADisplayData UMASkillModule::ResolveDisplayData(const UMAModuleQualityData* ModuleQualityData) const
{
	FMADisplayData DisplayData;
	DisplayData.DisplayName = ModuleData.DisplayData.DisplayName;
	DisplayData.Description = ModuleData.DisplayData.Description;
	DisplayData.IconData.Icon = ModuleData.DisplayData.IconData.Icon;
	DisplayData.IconData.SubIcon = ModuleData.AssembledSubIcon;
	if (!ModuleQualityData) return DisplayData;

	if (const FMAModuleTypeData* VisualData = ModuleQualityData->FindVisualData(ModuleData.ModuleVisualTags))
	{
		DisplayData.IconData.IconColor = VisualData->IconColor;
		DisplayData.IconData.InnerColor = VisualData->InnerColor;
	}
	if (const FMAModuleRarityData* RarityData =
		ModuleQualityData->FindRarityData(ModuleData.ModuleQuality.Rarity))
	{
		DisplayData.IconData.FrameColor = RarityData->Color;
	}
	return DisplayData;
}

FGameplayTag UMASkillModule::GetVisualElementTag() const
{
	static const FGameplayTag ElementalVisualTag = FGameplayTag::RequestGameplayTag(TEXT("Module.Visual.Elemental"));
	for (const FGameplayTag& VisualTag : ModuleData.ModuleVisualTags)
	{
		if (VisualTag != ElementalVisualTag && VisualTag.MatchesTag(ElementalVisualTag))
		{
			return VisualTag;
		}
	}

	return FGameplayTag();
}

const UMASkillModuleStackAddon* UMASkillModule::GetStackAddon() const
{
	return FindAddon<UMASkillModuleStackAddon>();
}

float UMASkillModule::GetCooldownSeconds() const
{
	const UMASkillCooldownAddon* CooldownAddon = FindAddon<UMASkillCooldownAddon>();
	return CooldownAddon ? CooldownAddon->GetCooldownSeconds() : 0.f;
}

void UMASkillModule::InitializeAddonRuntimeData(FMASkillModuleAddonRuntimeData& RuntimeData) const
{
	ForEachAddon([&](const UMASkillModuleAddon& Addon)
	{
		Addon.InitializeRuntimeData(RuntimeData);
	});
}

void UMASkillModule::ApplyAddonPayloadMirrors(
	const FMASkillModuleAddonRuntimeData& RuntimeData,
	FMASkillPayloadStore& PayloadStore) const
{
	ForEachAddon([&](const UMASkillModuleAddon& Addon)
	{
		Addon.ApplyPayloadMirror(RuntimeData, PayloadStore);
	});
}

void UMASkillModule::ForEachAddon(TFunctionRef<void(const UMASkillModuleAddon&)> Func) const
{
	TSet<const UClass*> SeenAddonClasses;
	for (const TObjectPtr<UMASkillModuleAddon>& Addon : ModuleData.Addons)
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

bool UMASkillModule::TryResolveSocketText(
	const FMASkillModuleAddonRuntimeData& RuntimeData,
	FText& OutText) const
{
	bool bResolved = false;
	ForEachAddon([&](const UMASkillModuleAddon& Addon)
	{
		if (!bResolved)
		{
			bResolved = Addon.TryResolveSocketText(RuntimeData, OutText);
		}
	});
	return bResolved;
}

void UMASkillModule::ApplyPayloadsTo(FMASkillPayloadStore& PayloadStore) const
{
	for (const FMASkillPayloadEntry& PayloadEntry : ModuleData.Payloads)
	{
		PayloadEntry.ApplyTo(PayloadStore);
	}
}

FMASkillModuleData& UMASkillModule::BeginAssembly()
{
	checkf(!IsAsset(), TEXT("Built skill module assets cannot be modified by runtime assembly."));
	ModuleId = 0;
	ModuleData = FMASkillModuleData();
	return ModuleData;
}
#if WITH_EDITOR
void UMASkillModule::SetGeneratedData(
	const int32 InModuleId,
	FMASkillModuleData&& InModuleData,
	FString&& InSourceHash,
	const int32 InGeneratedDataVersion,
	const int64 InLastBuiltAt)
{
	ModuleId = InModuleId;
	ModuleData = MoveTemp(InModuleData);
	SourceHash = MoveTemp(InSourceHash);
	GeneratedDataVersion = InGeneratedDataVersion;
	LastBuiltAt = InLastBuiltAt >= 0 ? InLastBuiltAt : FDateTime::UtcNow().ToUnixTimestamp();
}
#endif
