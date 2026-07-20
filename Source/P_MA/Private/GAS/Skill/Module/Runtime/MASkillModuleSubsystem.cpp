#include "GAS/Skill/Module/Runtime/MASkillModuleSubsystem.h"

#include "Engine/AssetManager.h"
#include "GAS/Skill/Module/MASkillModuleAsset.h"

void UMASkillModuleSubsystem::LoadModule(const int32 ModuleId, FMASkillModuleLoaded Completion)
{
	const FPrimaryAssetId AssetId = UMASkillModuleAsset::MakePrimaryAssetId(ModuleId);
	if (!AssetId.IsValid())
	{
		Completion.ExecuteIfBound(nullptr);
		return;
	}

	UAssetManager& AssetManager = UAssetManager::Get();
	if (const UMASkillModuleAsset* LoadedAsset =
		AssetManager.GetPrimaryAssetObject<UMASkillModuleAsset>(AssetId))
	{
		Completion.ExecuteIfBound(LoadedAsset->GetModuleId() == ModuleId ? LoadedAsset : nullptr);
		return;
	}

	TArray<FMASkillModuleLoaded>& Completions = PendingLoads.FindOrAdd(ModuleId);
	Completions.Add(MoveTemp(Completion));
	if (Completions.Num() > 1) return;

	AssetManager.LoadPrimaryAsset(
		AssetId,
		{},
		FStreamableDelegate::CreateUObject(this, &ThisClass::CompleteLoad, ModuleId));
}

void UMASkillModuleSubsystem::CompleteLoad(const int32 ModuleId)
{
	TArray<FMASkillModuleLoaded> Completions;
	PendingLoads.RemoveAndCopyValue(ModuleId, Completions);

	const UMASkillModuleAsset* LoadedAsset =
		UAssetManager::Get().GetPrimaryAssetObject<UMASkillModuleAsset>(
			UMASkillModuleAsset::MakePrimaryAssetId(ModuleId));
	if (LoadedAsset && LoadedAsset->GetModuleId() != ModuleId)
	{
		LoadedAsset = nullptr;
	}

	for (FMASkillModuleLoaded& Completion : Completions)
	{
		Completion.ExecuteIfBound(LoadedAsset);
	}
}
