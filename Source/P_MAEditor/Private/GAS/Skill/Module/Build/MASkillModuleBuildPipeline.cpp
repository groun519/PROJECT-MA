#include "GAS/Skill/Module/Build/MASkillModuleBuildPipeline.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManagerSettings.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/Build/MASkillModuleAssetBuilder.h"
#include "GAS/Skill/Module/Json/MASkillModuleJsonFile.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "ObjectTools.h"

static bool Fail(FText& OutError, const FString& Message)
{
	OutError = FText::FromString(Message);
	return false;
}

static FSoftObjectPath MakeGeneratedAssetPath(const FString& GeneratedAssetDirectory, const int32 ModuleId)
{
	const FString PackageName =
		FMASkillModuleAssetBuilder::MakeAssetPackageName(GeneratedAssetDirectory, ModuleId);
	return FSoftObjectPath(PackageName + TEXT(".") + FPaths::GetCleanFilename(PackageName));
}

static FString NormalizeSourceFile(const FString& SourceFile)
{
	FString NormalizedFile = FPaths::ConvertRelativePathToFull(SourceFile);
	FPaths::NormalizeFilename(NormalizedFile);
	return NormalizedFile;
}

static bool CollectSourceItems(
	const FString& SourceDirectory,
	TArray<FMASkillModuleBuildItem>& OutItems,
	TMap<int32, TArray<int32>>& OutSourceIndicesByModuleId,
	FText& OutError)
{
	OutItems.Reset();
	OutSourceIndicesByModuleId.Reset();
	OutError = FText::GetEmpty();

	const FString FullSourceDirectory = FPaths::ConvertRelativePathToFull(SourceDirectory);
	if (!IFileManager::Get().DirectoryExists(*FullSourceDirectory))
	{
		return Fail(OutError, FString::Printf(
			TEXT("Skill module source directory does not exist: %s"),
			*FullSourceDirectory));
	}

	TArray<FString> JsonFiles;
	IFileManager::Get().FindFilesRecursive(JsonFiles, *FullSourceDirectory, TEXT("*.json"), true, false);
	JsonFiles.Sort();

	for (const FString& JsonFile : JsonFiles)
	{
		FMASkillModuleBuildItem& Item = OutItems.AddDefaulted_GetRef();
		Item.SourceFile = NormalizeSourceFile(JsonFile);

		FMASkillModuleJsonHeader Header;
		FText HeaderError;
		const bool bHeaderValid = FMASkillModuleJsonFile::ReadHeader(
			JsonFile,
			Header,
			HeaderError);

		FText ItemError;
		if (!FMASkillModuleJsonFile::ResolveModuleId(JsonFile, Item.ModuleId, ItemError))
		{
			if (bHeaderValid)
			{
				Item.ModuleId = Header.ModuleId;
				Item.ModuleType = Header.ModuleType;
				OutSourceIndicesByModuleId.FindOrAdd(Header.ModuleId).Add(OutItems.Num() - 1);
				Item.StatusDetail = MoveTemp(ItemError);
			}
			else
			{
				Item.StatusDetail = FText::FromString(FString::Printf(
					TEXT("%s %s"),
					*ItemError.ToString(),
					*HeaderError.ToString()));
			}
			continue;
		}
		OutSourceIndicesByModuleId.FindOrAdd(Item.ModuleId).Add(OutItems.Num() - 1);
		if (!bHeaderValid)
		{
			Item.StatusDetail = MoveTemp(HeaderError);
			continue;
		}

		Item.ModuleType = Header.ModuleType;
		if (Header.ModuleId == Item.ModuleId
			&& !FMASkillModuleJsonFile::ValidateSourceFilePath(
				FullSourceDirectory,
				Item.SourceFile,
				Item.ModuleType,
				ItemError))
		{
			Item.StatusDetail = MoveTemp(ItemError);
		}
	}

	for (const TPair<int32, TArray<int32>>& Pair : OutSourceIndicesByModuleId)
	{
		if (Pair.Value.Num() <= 1) continue;

		for (const int32 ItemIndex : Pair.Value)
		{
			FMASkillModuleBuildItem& Item = OutItems[ItemIndex];
			FMASkillModuleJsonHeader Header;
			FText ContentError;
			if (!FMASkillModuleJsonFile::ReadHeader(
				Item.SourceFile,
				Header,
				ContentError)
				|| Header.ModuleId != Pair.Key)
			{
				Item.StatusDetail = ContentError.IsEmpty()
					? FText::FromString(FString::Printf(
						TEXT("Source ModuleId %d does not match file ModuleId %d."),
						Header.ModuleId,
						Pair.Key))
					: MoveTemp(ContentError);
				continue;
			}

			Item.StatusDetail = FText::FromString(FString::Printf(
				TEXT("Multiple source files use ModuleId %d."),
				Pair.Key));
		}
	}
	return true;
}

bool FMASkillModuleBuildPipeline::CollectStatus(
	const FString& SourceDirectory,
	TArray<FMASkillModuleBuildItem>& OutItems,
	FText& OutError)
{
	TMap<int32, TArray<int32>> SourceIndicesByModuleId;
	if (!CollectSourceItems(SourceDirectory, OutItems, SourceIndicesByModuleId, OutError)) return false;

	FString GeneratedAssetDirectory;
	if (!ResolveGeneratedAssetDirectory(GeneratedAssetDirectory, OutError)) return false;

	for (FMASkillModuleBuildItem& Item : OutItems)
	{
		if (Item.ModuleId <= 0
			|| !Item.StatusDetail.IsEmpty())
		{
			continue;
		}

		FMASkillModuleBuildItem InspectedItem;
		FText ItemError;
		const bool bResolved = FMASkillModuleAssetBuilder::ResolveBuildStatus(
			Item.SourceFile,
			GeneratedAssetDirectory,
			InspectedItem,
			ItemError);
		InspectedItem.ModuleType = Item.ModuleType;
		Item = MoveTemp(InspectedItem);
		if (!bResolved)
		{
			Item.Status = EMASkillModuleBuildStatus::Error;
			Item.StatusDetail = MoveTemp(ItemError);
		}
	}

	FARFilter Filter;
	Filter.PackagePaths.Add(*GeneratedAssetDirectory);
	Filter.ClassPaths.Add(UMASkillModule::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;

	FAssetRegistryModule& AssetRegistry =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> GeneratedAssets;
	AssetRegistry.Get().GetAssets(Filter, GeneratedAssets);
	for (const FAssetData& AssetData : GeneratedAssets)
	{
		if (!FPackageName::DoesPackageExist(AssetData.PackageName.ToString())) continue;

		FMASkillModuleBuildItem Item;
		Item.GeneratedAssetPath = AssetData.GetSoftObjectPath();
		AssetData.GetTagValue(UMASkillModule::GetLastBuiltAtTag(), Item.LastBuiltAt);
		if (!AssetData.GetTagValue(UMASkillModule::GetModuleIdTag(), Item.ModuleId)
			|| Item.ModuleId <= 0)
		{
			Item.StatusDetail = FText::FromString(TEXT("Generated asset has an invalid ModuleId."));
			OutItems.Add(MoveTemp(Item));
			continue;
		}

		const FSoftObjectPath ExpectedAssetPath = MakeGeneratedAssetPath(GeneratedAssetDirectory, Item.ModuleId);
		if (const TArray<int32>* SourceIndices = SourceIndicesByModuleId.Find(Item.ModuleId))
		{
			if (Item.GeneratedAssetPath != ExpectedAssetPath)
			{
				Item.StatusDetail = FText::FromString(FString::Printf(
					TEXT("Generated asset must be located at '%s'."),
					*ExpectedAssetPath.ToString()));
				OutItems.Add(MoveTemp(Item));
			}
			else
			{
				for (const int32 SourceIndex : *SourceIndices)
				{
					OutItems[SourceIndex].GeneratedAssetPath = Item.GeneratedAssetPath;
					OutItems[SourceIndex].LastBuiltAt = Item.LastBuiltAt;
				}
			}
			continue;
		}

		Item.StatusDetail = FText::FromString(TEXT("Source JSON does not exist."));
		OutItems.Add(MoveTemp(Item));
	}

	return true;
}

bool FMASkillModuleBuildPipeline::BuildFile(
	const FString& SourceDirectory,
	const FString& JsonFile,
	FMASkillModuleBuildSummary& OutSummary,
	FText& OutError)
{
	const TArray<FString> JsonFiles{JsonFile};
	return BuildFiles(
		SourceDirectory,
		JsonFiles,
		EMASkillModuleBuildMode::IfRequired,
		OutSummary,
		OutError);
}

bool FMASkillModuleBuildPipeline::BuildFiles(
	const FString& SourceDirectory,
	const TArray<FString>& JsonFiles,
	const EMASkillModuleBuildMode BuildMode,
	FMASkillModuleBuildSummary& OutSummary,
	FText& OutError)
{
	OutSummary = FMASkillModuleBuildSummary();
	OutError = FText::GetEmpty();

	TArray<FMASkillModuleBuildItem> SourceItems;
	TMap<int32, TArray<int32>> SourceIndicesByModuleId;
	if (!CollectSourceItems(SourceDirectory, SourceItems, SourceIndicesByModuleId, OutError)) return false;

	TMap<FString, const FMASkillModuleBuildItem*> SourceItemsByFile;
	SourceItemsByFile.Reserve(SourceItems.Num());
	for (const FMASkillModuleBuildItem& Item : SourceItems)
	{
		SourceItemsByFile.Add(Item.SourceFile, &Item);
	}

	FString GeneratedAssetDirectory;
	if (!ResolveGeneratedAssetDirectory(GeneratedAssetDirectory, OutError)) return false;

	for (const FString& JsonFile : JsonFiles)
	{
		const FMASkillModuleBuildItem* const* SourceItem =
			SourceItemsByFile.Find(NormalizeSourceFile(JsonFile));
		if (!SourceItem)
		{
			OutSummary.Failures.Add({JsonFile, FText::FromString(
				TEXT("Source JSON is not registered in the configured JSON directory."))});
			continue;
		}
		if (!(*SourceItem)->StatusDetail.IsEmpty())
		{
			OutSummary.Failures.Add({JsonFile, (*SourceItem)->StatusDetail});
			continue;
		}

		EMASkillModuleAssetBuildResult Result;
		FText BuildError;
		if (!FMASkillModuleAssetBuilder::Build(
			JsonFile,
			GeneratedAssetDirectory,
			BuildMode,
			Result,
			BuildError))
		{
			OutSummary.Failures.Add({JsonFile, MoveTemp(BuildError)});
			continue;
		}

		if (Result == EMASkillModuleAssetBuildResult::Built) ++OutSummary.Built;
		else ++OutSummary.UpToDate;
	}

	if (OutSummary.Failures.IsEmpty()) return true;

	TArray<FString> FailureMessages;
	FailureMessages.Reserve(OutSummary.Failures.Num());
	for (const FMASkillModuleBuildFailure& Failure : OutSummary.Failures)
	{
		FailureMessages.Add(FString::Printf(
			TEXT("%s: %s"),
			*FPaths::GetCleanFilename(Failure.SourceFile),
			*Failure.Error.ToString()));
	}
	OutError = FText::FromString(FString::Join(FailureMessages, TEXT("\n")));
	return false;
}

bool FMASkillModuleBuildPipeline::DeleteGeneratedAsset(
	const FSoftObjectPath& GeneratedAsset,
	FText& OutError)
{
	OutError = FText::GetEmpty();
	if (!GeneratedAsset.IsValid()) return Fail(OutError, TEXT("No generated module asset was selected."));

	FAssetRegistryModule& AssetRegistry =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	const FAssetData AssetData = AssetRegistry.Get().GetAssetByObjectPath(GeneratedAsset);
	if (!AssetData.IsValid()
		|| !FPackageName::DoesPackageExist(AssetData.PackageName.ToString()))
	{
		return true;
	}
	if (AssetData.AssetClassPath != UMASkillModule::StaticClass()->GetClassPathName())
	{
		return Fail(OutError, TEXT("The selected asset is not a generated skill module asset."));
	}

	const TArray<FAssetData> AssetsToDelete{AssetData};
	return ObjectTools::DeleteAssets(AssetsToDelete, false) == 1
		? true
		: Fail(OutError, TEXT("Failed to delete the generated skill module asset."));
}

bool FMASkillModuleBuildPipeline::ResolveNextModuleId(
	const FString& SourceDirectory,
	int32& OutModuleId,
	FText& OutError)
{
	OutModuleId = 0;
	OutError = FText::GetEmpty();
	int32 MaxModuleId = 0;

	TArray<FString> JsonFiles;
	IFileManager::Get().FindFilesRecursive(JsonFiles, *SourceDirectory, TEXT("*.json"), true, false);
	for (const FString& JsonFile : JsonFiles)
	{
		int32 ModuleId = 0;
		FText IgnoredError;
		if (FMASkillModuleJsonFile::ResolveModuleId(JsonFile, ModuleId, IgnoredError))
		{
			MaxModuleId = FMath::Max(MaxModuleId, ModuleId);
		}
		if (FMASkillModuleJsonFile::ResolveModuleIdFromContent(JsonFile, ModuleId, IgnoredError))
		{
			MaxModuleId = FMath::Max(MaxModuleId, ModuleId);
		}
	}

	FString GeneratedAssetDirectory;
	if (!ResolveGeneratedAssetDirectory(GeneratedAssetDirectory, OutError)) return false;

	FARFilter Filter;
	Filter.PackagePaths.Add(*GeneratedAssetDirectory);
	Filter.ClassPaths.Add(UMASkillModule::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;
	FAssetRegistryModule& AssetRegistry =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> GeneratedAssets;
	AssetRegistry.Get().GetAssets(Filter, GeneratedAssets);
	for (const FAssetData& AssetData : GeneratedAssets)
	{
		if (!FPackageName::DoesPackageExist(AssetData.PackageName.ToString())) continue;

		int32 ModuleId = 0;
		if (AssetData.GetTagValue(UMASkillModule::GetModuleIdTag(), ModuleId))
		{
			MaxModuleId = FMath::Max(MaxModuleId, ModuleId);
		}
		FText IgnoredError;
		if (FMASkillModuleJsonFile::ResolveModuleId(
			AssetData.AssetName.ToString() + TEXT(".json"),
			ModuleId,
			IgnoredError))
		{
			MaxModuleId = FMath::Max(MaxModuleId, ModuleId);
		}
	}

	if (MaxModuleId == MAX_int32)
	{
		return Fail(OutError, TEXT("No ModuleId remains available."));
	}

	OutModuleId = MaxModuleId + 1;
	return true;
}

bool FMASkillModuleBuildPipeline::ResolveGeneratedAssetDirectory(
	FString& OutDirectory,
	FText& OutError)
{
	OutDirectory.Reset();
	OutError = FText::GetEmpty();

	const UAssetManagerSettings* Settings = GetDefault<UAssetManagerSettings>();
	const FPrimaryAssetTypeInfo* TypeInfo = Settings->PrimaryAssetTypesToScan.FindByPredicate(
		[](const FPrimaryAssetTypeInfo& Candidate)
		{
			return Candidate.PrimaryAssetType == UMASkillModule::PrimaryAssetType;
		});
	if (!TypeInfo)
	{
		return Fail(OutError, TEXT("Asset Manager does not define the SkillModule primary asset type."));
	}
	if (TypeInfo->GetDirectories().Num() != 1)
	{
		return Fail(OutError, TEXT("SkillModule primary assets require exactly one generated asset directory."));
	}

	OutDirectory = TypeInfo->GetDirectories()[0].Path;
	FPaths::NormalizeDirectoryName(OutDirectory);
	FText PathError;
	if (!FPackageName::IsValidLongPackageName(OutDirectory, false, &PathError))
	{
		return Fail(OutError, FString::Printf(
			TEXT("Invalid SkillModule generated asset directory '%s': %s"),
			*OutDirectory,
			*PathError.ToString()));
	}

	return true;
}
