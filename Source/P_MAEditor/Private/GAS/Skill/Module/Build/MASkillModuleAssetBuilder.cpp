#include "GAS/Skill/Module/Build/MASkillModuleAssetBuilder.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "GAS/Skill/Module/MASkillModuleAsset.h"
#include "GAS/Skill/Module/Json/MASkillModuleJsonFile.h"
#include "GAS/Skill/Module/Json/MASkillModuleJsonReader.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "UObject/StrongObjectPtr.h"

struct FMASkillModuleAddonMove
{
	UMASkillModuleAddon* Addon = nullptr;
	UObject* OriginalOuter = nullptr;
	FName OriginalName;
};

static bool Fail(FText& OutError, const FString& Message)
{
	OutError = FText::FromString(Message);
	return false;
}

static bool MoveAddon(
	UMASkillModuleAddon& Addon,
	UObject& NewOuter,
	const FName NewName,
	TArray<FMASkillModuleAddonMove>& OutMoves,
	FText& OutError)
{
	const FMASkillModuleAddonMove Move{&Addon, Addon.GetOuter(), Addon.GetFName()};
	if (!Addon.Rename(
		*NewName.ToString(),
		&NewOuter,
		REN_DoNotDirty | REN_DontCreateRedirectors | REN_NonTransactional))
	{
		return Fail(OutError, FString::Printf(
			TEXT("Failed to move addon '%s' to '%s'."),
			*Addon.GetPathName(),
			*NewOuter.GetPathName()));
	}

	OutMoves.Add(Move);
	return true;
}

static bool RestoreMovedAddons(TArray<FMASkillModuleAddonMove>& Moves, FText& OutError)
{
	bool bRestored = true;
	for (int32 Index = Moves.Num() - 1; Index >= 0; --Index)
	{
		const FMASkillModuleAddonMove& Move = Moves[Index];
		if (!Move.Addon->Rename(
			*Move.OriginalName.ToString(),
			Move.OriginalOuter,
			REN_DoNotDirty | REN_DontCreateRedirectors | REN_NonTransactional))
		{
			if (bRestored)
			{
				OutError = FText::FromString(FString::Printf(
					TEXT("Failed to restore addon '%s'."),
					*Move.Addon->GetPathName()));
			}
			bRestored = false;
		}
	}

	if (bRestored) Moves.Reset();
	return bRestored;
}

static EMASkillModuleBuildStatus EvaluateBuildStatus(
	const FAssetData& AssetData,
	const int32 ModuleId,
	const FString& SourceHash,
	FText& OutDetail)
{
	OutDetail = FText::GetEmpty();
	if (!AssetData.IsValid()
		|| !FPackageName::DoesPackageExist(AssetData.PackageName.ToString()))
	{
		OutDetail = FText::FromString(TEXT("Generated asset does not exist."));
		return EMASkillModuleBuildStatus::NeedsBuild;
	}
	if (AssetData.AssetClassPath != UMASkillModuleAsset::StaticClass()->GetClassPathName())
	{
		OutDetail = FText::FromString(FString::Printf(
			TEXT("Generated asset path is occupied by '%s'."),
			*AssetData.AssetClassPath.ToString()));
		return EMASkillModuleBuildStatus::Error;
	}

	int32 ExistingModuleId = 0;
	if (!AssetData.GetTagValue(UMASkillModuleAsset::GetModuleIdTag(), ExistingModuleId)
		|| ExistingModuleId != ModuleId)
	{
		OutDetail = FText::FromString(TEXT("Generated asset has an invalid ModuleId."));
		return EMASkillModuleBuildStatus::Error;
	}

	int32 BuiltVersion = 0;
	if (!AssetData.GetTagValue(UMASkillModuleAsset::GetGeneratedDataVersionTag(), BuiltVersion)
		|| BuiltVersion != UMASkillModuleAsset::CurrentGeneratedDataVersion)
	{
		OutDetail = FText::FromString(FString::Printf(
			TEXT("Generated data version %d must be rebuilt as version %d."),
			BuiltVersion,
			UMASkillModuleAsset::CurrentGeneratedDataVersion));
		return EMASkillModuleBuildStatus::NeedsBuild;
	}
	FString ExistingHash;
	if (!AssetData.GetTagValue(UMASkillModuleAsset::GetSourceHashTag(), ExistingHash)
		|| ExistingHash.IsEmpty()
		|| ExistingHash != SourceHash)
	{
		OutDetail = FText::FromString(TEXT("Source JSON has changed since the last build."));
		return EMASkillModuleBuildStatus::NeedsBuild;
	}

	return EMASkillModuleBuildStatus::Built;
}

bool FMASkillModuleAssetBuilder::ResolveBuildStatus(
	const FString& JsonFile,
	const FString& GeneratedAssetDirectory,
	FMASkillModuleBuildItem& OutItem,
	FText& OutError)
{
	OutItem = FMASkillModuleBuildItem();
	OutItem.SourceFile = JsonFile;
	OutError = FText::GetEmpty();

	FText PathError;
	if (!FPackageName::IsValidLongPackageName(GeneratedAssetDirectory, false, &PathError))
	{
		return Fail(OutError, FString::Printf(
			TEXT("Invalid generated asset directory '%s': %s"),
			*GeneratedAssetDirectory,
			*PathError.ToString()));
	}
	FMASkillModuleJsonSource Source;
	if (!FMASkillModuleJsonFile::Load(JsonFile, Source, OutError)) return false;
	OutItem.ModuleId = Source.ModuleId;

	const FString PackageName = MakeAssetPackageName(GeneratedAssetDirectory, OutItem.ModuleId);
	const FString AssetName = FPaths::GetCleanFilename(PackageName);
	const FSoftObjectPath GeneratedAssetPath(PackageName + TEXT(".") + AssetName);

	FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	const FAssetData AssetData = AssetRegistry.Get().GetAssetByObjectPath(GeneratedAssetPath);
	if (AssetData.IsValid()
		&& FPackageName::DoesPackageExist(AssetData.PackageName.ToString())
		&& AssetData.AssetClassPath == UMASkillModuleAsset::StaticClass()->GetClassPathName())
	{
		OutItem.GeneratedAssetPath = AssetData.GetSoftObjectPath();
	}
	AssetData.GetTagValue(UMASkillModuleAsset::GetLastBuiltAtTag(), OutItem.LastBuiltAt);
	OutItem.Status = EvaluateBuildStatus(
		AssetData,
		OutItem.ModuleId,
		Source.SourceHash,
		OutItem.StatusDetail);
	if (OutItem.Status != EMASkillModuleBuildStatus::NeedsBuild) return true;

	TStrongObjectPtr<UMASkillModuleAsset> ValidationOwner(NewObject<UMASkillModuleAsset>());
	const FMASkillModuleReadResult ReadResult = FMASkillModuleJsonReader::Read(Source, *ValidationOwner);
	if (!ReadResult.IsValid())
	{
		OutItem.Status = EMASkillModuleBuildStatus::Error;
		OutItem.StatusDetail = ReadResult.GetDiagnosticsText();
	}
	return true;
}

bool FMASkillModuleAssetBuilder::Build(
	const FString& JsonFile,
	const FString& GeneratedAssetDirectory,
	const EMASkillModuleBuildMode BuildMode,
	EMASkillModuleAssetBuildResult& OutResult,
	FText& OutError)
{
	OutResult = EMASkillModuleAssetBuildResult::Built;
	OutError = FText::GetEmpty();

	FText PathError;
	if (!FPackageName::IsValidLongPackageName(GeneratedAssetDirectory, false, &PathError))
	{
		return Fail(OutError, FString::Printf(
			TEXT("Invalid generated asset directory '%s': %s"),
			*GeneratedAssetDirectory,
			*PathError.ToString()));
	}

	FMASkillModuleJsonSource Source;
	if (!FMASkillModuleJsonFile::Load(JsonFile, Source, OutError)) return false;
	const int32 ModuleId = Source.ModuleId;

	const FString PackageName = MakeAssetPackageName(GeneratedAssetDirectory, ModuleId);
	const FString AssetName = FPaths::GetCleanFilename(PackageName);
	const FSoftObjectPath AssetPath(PackageName + TEXT(".") + AssetName);
	FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	const FAssetData ExistingAssetData = AssetRegistry.Get().GetAssetByObjectPath(AssetPath);
	FText StatusDetail;
	const EMASkillModuleBuildStatus BuildStatus =
		EvaluateBuildStatus(
			ExistingAssetData,
			ModuleId,
			Source.SourceHash,
			StatusDetail);
	if (BuildStatus == EMASkillModuleBuildStatus::Error)
	{
		return Fail(OutError, FString::Printf(
			TEXT("Cannot build generated asset '%s': %s"),
			*AssetPath.ToString(),
			*StatusDetail.ToString()));
	}
	if (BuildMode == EMASkillModuleBuildMode::IfRequired
		&& BuildStatus == EMASkillModuleBuildStatus::Built)
	{
		OutResult = EMASkillModuleAssetBuildResult::UpToDate;
		return true;
	}

	TStrongObjectPtr<UMASkillModuleAsset> BuildCandidate(NewObject<UMASkillModuleAsset>());
	FMASkillModuleReadResult ReadResult = FMASkillModuleJsonReader::Read(Source, *BuildCandidate);
	if (!ReadResult.IsValid())
	{
		return Fail(OutError, FString::Printf(
			TEXT("Failed to build skill module JSON '%s': %s"),
			*JsonFile,
			*ReadResult.GetDiagnosticsText().ToString()));
	}
	FMASkillModuleData ModuleData = MoveTemp(ReadResult.ModuleData);

	const FString PackageFilename =
		FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	const bool bPackageExists = FPackageName::DoesPackageExist(PackageName);
	if (bPackageExists && IFileManager::Get().IsReadOnly(*PackageFilename))
	{
		return Fail(OutError, FString::Printf(TEXT("Generated module asset is read-only: %s"), *PackageFilename));
	}
	if (!IFileManager::Get().MakeDirectory(*FPaths::GetPath(PackageFilename), true))
	{
		return Fail(OutError, FString::Printf(TEXT("Failed to create generated asset directory: %s"), *PackageFilename));
	}
	for (UMASkillModuleAddon* Addon : ModuleData.Addons)
	{
		Addon->BuildGeneratedData();
	}

	UMASkillModuleAsset* TargetAsset = ExistingAssetData.IsValid()
		? Cast<UMASkillModuleAsset>(ExistingAssetData.GetAsset())
		: nullptr;
	if (!TargetAsset && bPackageExists)
	{
		TargetAsset = LoadObject<UMASkillModuleAsset>(nullptr, *AssetPath.ToString());
	}
	if (bPackageExists && !TargetAsset)
	{
		return Fail(OutError, FString::Printf(
			TEXT("Generated package does not contain the requested module asset: %s"),
			*AssetPath.ToString()));
	}

	UPackage* Package = TargetAsset ? TargetAsset->GetPackage() : CreatePackage(*PackageName);
	const bool bNewAsset = !TargetAsset;
	if (bNewAsset)
	{
		TargetAsset = NewObject<UMASkillModuleAsset>(
			Package,
			*AssetName,
			RF_Public | RF_Standalone | RF_Transactional);
	}

	const bool bWasDirty = Package->IsDirty();
	const int32 PreviousModuleId = TargetAsset->GetModuleId();
	FMASkillModuleData PreviousModuleData = TargetAsset->GetModuleData();
	FString PreviousSourceHash = TargetAsset->GetSourceHash();
	const int32 PreviousVersion = TargetAsset->GetGeneratedDataVersion();
	const int64 PreviousLastBuiltAt = TargetAsset->GetLastBuiltAt();
	TStrongObjectPtr<UMASkillModuleAsset> PreviousAddonOwner(NewObject<UMASkillModuleAsset>());
	TArray<FMASkillModuleAddonMove> AddonMoves;

	for (UMASkillModuleAddon* Addon : PreviousModuleData.Addons)
	{
		const FName StagingName = MakeUniqueObjectName(
			PreviousAddonOwner.Get(),
			Addon->GetClass(),
			Addon->GetFName());
		if (!MoveAddon(*Addon, *PreviousAddonOwner, StagingName, AddonMoves, OutError))
		{
			RestoreMovedAddons(AddonMoves, OutError);
			return false;
		}
	}

	for (UMASkillModuleAddon* Addon : ModuleData.Addons)
	{
		const FName AddonName(*FString::Printf(
			TEXT("M%d_%s"),
			ModuleId,
			*Addon->GetClass()->GetName()));
		if (!MoveAddon(*Addon, *TargetAsset, AddonName, AddonMoves, OutError))
		{
			RestoreMovedAddons(AddonMoves, OutError);
			return false;
		}
	}

	TargetAsset->SetGeneratedData(ModuleId, MoveTemp(ModuleData), MoveTemp(Source.SourceHash));
	Package->MarkPackageDirty();
	if (bNewAsset) FAssetRegistryModule::AssetCreated(TargetAsset);

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;
	if (!UPackage::SavePackage(Package, TargetAsset, *PackageFilename, SaveArgs))
	{
		Fail(OutError, FString::Printf(TEXT("Failed to save generated module asset: %s"), *PackageFilename));
		if (bNewAsset) FAssetRegistryModule::AssetDeleted(TargetAsset);
		RestoreMovedAddons(AddonMoves, OutError);
		TargetAsset->SetGeneratedData(
			PreviousModuleId,
			MoveTemp(PreviousModuleData),
			MoveTemp(PreviousSourceHash),
			PreviousVersion,
			PreviousLastBuiltAt);
		if (bNewAsset)
		{
			const FName TransientName = MakeUniqueObjectName(
				GetTransientPackage(),
				TargetAsset->GetClass(),
				TargetAsset->GetFName());
			TargetAsset->ClearFlags(RF_Public | RF_Standalone);
			TargetAsset->Rename(
				*TransientName.ToString(),
				GetTransientPackage(),
				REN_DoNotDirty | REN_DontCreateRedirectors | REN_NonTransactional);
		}
		Package->SetDirtyFlag(bWasDirty);
		return false;
	}

	TArray<FAssetData> SavedAssets;
	SavedAssets.Emplace(TargetAsset);
	FAssetRegistryModule::AssetsSaved(MoveTemp(SavedAssets));
	Package->SetDirtyFlag(false);
	return true;
}

FString FMASkillModuleAssetBuilder::MakeAssetPackageName(
	const FString& GeneratedAssetDirectory,
	const int32 ModuleId)
{
	return GeneratedAssetDirectory / FString::Printf(TEXT("M_%d"), ModuleId);
}
