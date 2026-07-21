#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/Build/MASkillModuleAssetBuilder.h"
#include "GAS/Skill/Module/Build/MASkillModuleBuildPipeline.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "ObjectTools.h"
#include "UObject/Linker.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMASkillModuleBuildPipelineTest,
	"P_MA.Skill.ModuleAsset.Pipeline",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMASkillModuleBuildPipelineTest::RunTest(const FString& Parameters)
{
	const FString TestId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	const int32 ModuleId = 100000000 + static_cast<int32>(GetTypeHash(TestId) % 1000000000);
	const FString SourceDirectory = FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("Automation/SkillModulePipeline"),
		TestId);
	const FString JsonFile = FPaths::Combine(
		SourceDirectory,
		FString::Printf(TEXT("M_%d.json"), ModuleId));
	const FString InvalidJsonFile = FPaths::Combine(
		SourceDirectory,
		FString::Printf(TEXT("M_%d.json"), ModuleId + 1));
	const FString InvalidNameJsonFile = FPaths::Combine(SourceDirectory, TEXT("InvalidName.json"));
	const FString ValidInvalidNameJsonFile = FPaths::Combine(SourceDirectory, TEXT("ValidInvalidName.json"));
	const FString BuiltInvalidNameJsonFile = FPaths::Combine(SourceDirectory, TEXT("BuiltInvalidName.json"));
	const int32 DuplicateModuleId = ModuleId + 10;
	const FString DuplicateJsonFile = FPaths::Combine(
		SourceDirectory,
		FString::Printf(TEXT("M_%d.json"), DuplicateModuleId));
	const FString DuplicateJsonFile2 = FPaths::Combine(
		SourceDirectory,
		TEXT("Duplicate"),
		FString::Printf(TEXT("M_%d.json"), DuplicateModuleId));

	FString GeneratedDirectory;
	FText Error;
	if (!TestTrue(
		TEXT("Resolve generated asset directory"),
		FMASkillModuleBuildPipeline::ResolveGeneratedAssetDirectory(GeneratedDirectory, Error)))
	{
		AddError(Error.ToString());
		return false;
	}

	const FString PackageName =
		FMASkillModuleAssetBuilder::MakeAssetPackageName(GeneratedDirectory, ModuleId);
	const FString AssetName = FPaths::GetCleanFilename(PackageName);
	const FSoftObjectPath AssetPath(PackageName + TEXT(".") + AssetName);

	ON_SCOPE_EXIT
	{
		for (const int32 CleanupModuleId : {ModuleId, DuplicateModuleId})
		{
			const FString CleanupPackageName =
				FMASkillModuleAssetBuilder::MakeAssetPackageName(GeneratedDirectory, CleanupModuleId);
			if (!FPackageName::DoesPackageExist(CleanupPackageName)) continue;

			const FString CleanupAssetName = FPaths::GetCleanFilename(CleanupPackageName);
			const FSoftObjectPath CleanupAssetPath(
				CleanupPackageName + TEXT(".") + CleanupAssetName);
			FAssetRegistryModule& AssetRegistry =
				FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
			const FAssetData AssetData = AssetRegistry.Get().GetAssetByObjectPath(CleanupAssetPath);
			if (AssetData.IsValid())
			{
				TArray<FAssetData> AssetsToDelete{AssetData};
				ObjectTools::DeleteAssets(AssetsToDelete, false);
			}
			else
			{
				if (UObject* Asset = CleanupAssetPath.ResolveObject())
				{
					Asset->GetPackage()->SetDirtyFlag(false);
					ResetLoaders(Asset->GetPackage());
				}
				const FString CleanupPackageFilename = FPackageName::LongPackageNameToFilename(
					CleanupPackageName,
					FPackageName::GetAssetPackageExtension());
				IFileManager::Get().Delete(*CleanupPackageFilename, false, true, true);
			}
		}
		IFileManager::Get().DeleteDirectory(*SourceDirectory, false, true);
	};

	if (!TestTrue(
		TEXT("Create source directory"),
		IFileManager::Get().MakeDirectory(*SourceDirectory, true)))
	{
		return false;
	}

	const FString Json = FString::Printf(
		TEXT("{\"ModuleId\":%d,\"Module\":{\"ModuleName\":\"Pipeline\"}}"),
		ModuleId);
	if (!TestTrue(TEXT("Write source JSON"), FFileHelper::SaveStringToFile(Json, *JsonFile)))
	{
		return false;
	}
	const FString DuplicateJson = FString::Printf(
		TEXT("{\"ModuleId\":%d,\"Module\":{}}"),
		DuplicateModuleId);
	TestTrue(
		TEXT("Create duplicate source directory"),
		IFileManager::Get().MakeDirectory(*FPaths::GetPath(DuplicateJsonFile2), true));
	TestTrue(
		TEXT("Write invalid-name source JSON"),
		FFileHelper::SaveStringToFile(TEXT("{}"), *InvalidNameJsonFile));
	TestTrue(
		TEXT("Write valid source with an invalid file name"),
		FFileHelper::SaveStringToFile(
			FString::Printf(
				TEXT("{\"ModuleId\":%d,\"Module\":{}}"),
				ModuleId + 20),
			*ValidInvalidNameJsonFile));
	TestTrue(
		TEXT("Write first duplicate source JSON"),
		FFileHelper::SaveStringToFile(DuplicateJson, *DuplicateJsonFile));
	TestTrue(
		TEXT("Write second duplicate source JSON"),
		FFileHelper::SaveStringToFile(DuplicateJson, *DuplicateJsonFile2));
	const FString MismatchedIdJsonFile = FPaths::Combine(
		SourceDirectory,
		FString::Printf(TEXT("M_%d.json"), ModuleId + 30));
	TestTrue(
		TEXT("Write source with mismatched file and content ids"),
		FFileHelper::SaveStringToFile(
			FString::Printf(TEXT("{\"ModuleId\":%d,\"Module\":{}}"), ModuleId + 40),
			*MismatchedIdJsonFile));
	int32 NextModuleId = 0;
	TestTrue(
		TEXT("Resolve next module id"),
		FMASkillModuleBuildPipeline::ResolveNextModuleId(SourceDirectory, NextModuleId, Error));
	TestTrue(
		TEXT("Next module id reserves both file and content ids"),
		NextModuleId > ModuleId + 40);
	IFileManager::Get().Delete(*MismatchedIdJsonFile);
	TArray<FMASkillModuleBuildItem> Items;
	if (!TestTrue(
		TEXT("Collect initial build status"),
		FMASkillModuleBuildPipeline::CollectStatus(SourceDirectory, Items, Error)))
	{
		AddError(Error.ToString());
		return false;
	}
	const FMASkillModuleBuildItem* Item = Items.FindByPredicate([ModuleId](const FMASkillModuleBuildItem& Candidate)
	{
		return Candidate.ModuleId == ModuleId && !Candidate.SourceFile.IsEmpty();
	});
	TestNotNull(TEXT("Initial status contains source module"), Item);
	if (!Item) return false;
	TestEqual(TEXT("Initial source requires build"), Item->Status, EMASkillModuleBuildStatus::NeedsBuild);
	const int32 InvalidSourceCount = Items.FilterByPredicate([](const FMASkillModuleBuildItem& Candidate)
	{
		return !Candidate.SourceFile.IsEmpty()
			&& Candidate.Status == EMASkillModuleBuildStatus::Error;
	}).Num();
	TestEqual(TEXT("Invalid and duplicate source files are reported"), InvalidSourceCount, 4);
	const FMASkillModuleBuildItem* InvalidNameItem = Items.FindByPredicate([](const FMASkillModuleBuildItem& Candidate)
	{
		return FPaths::GetCleanFilename(Candidate.SourceFile) == TEXT("InvalidName.json");
	});
	TestNotNull(TEXT("Invalid source name is classified"), InvalidNameItem);
	if (InvalidNameItem)
	{
		TestEqual(
			TEXT("Unreadable invalid-name source is an error"),
			InvalidNameItem->Status,
			EMASkillModuleBuildStatus::Error);
		TestFalse(TEXT("Unreadable source describes its error"), InvalidNameItem->StatusDetail.IsEmpty());
	}
	const FMASkillModuleBuildItem* NormalizableNameItem = Items.FindByPredicate([](
		const FMASkillModuleBuildItem& Candidate)
	{
		return FPaths::GetCleanFilename(Candidate.SourceFile) == TEXT("ValidInvalidName.json");
	});
	TestNotNull(TEXT("Valid source with an invalid name is classified"), NormalizableNameItem);
	if (NormalizableNameItem)
	{
		TestEqual(
			TEXT("Valid source with an invalid name is an error"),
			NormalizableNameItem->Status,
			EMASkillModuleBuildStatus::Error);
		TestFalse(TEXT("Invalid name describes its error"), NormalizableNameItem->StatusDetail.IsEmpty());
	}
	const FMASkillModuleBuildItem* DuplicateItem = Items.FindByPredicate([DuplicateModuleId](
		const FMASkillModuleBuildItem& Candidate)
	{
		return Candidate.ModuleId == DuplicateModuleId;
	});
	TestNotNull(TEXT("Duplicate source id is classified"), DuplicateItem);
	if (DuplicateItem)
	{
		TestEqual(
			TEXT("Duplicate source id is an error"),
			DuplicateItem->Status,
			EMASkillModuleBuildStatus::Error);
		TestFalse(TEXT("Duplicate source describes its error"), DuplicateItem->StatusDetail.IsEmpty());
	}
	FMASkillModuleBuildSummary DuplicateSummary;
	TestFalse(
		TEXT("Single build cannot bypass duplicate source validation"),
		FMASkillModuleBuildPipeline::BuildFile(
			SourceDirectory,
			DuplicateJsonFile,
			DuplicateSummary,
			Error));
	TestEqual(TEXT("Duplicate source is reported as a build failure"), DuplicateSummary.Failures.Num(), 1);

	FMASkillModuleBuildSummary Summary;
	if (!TestTrue(
		TEXT("Build source through pipeline"),
		FMASkillModuleBuildPipeline::BuildFile(SourceDirectory, JsonFile, Summary, Error)))
	{
		AddError(Error.ToString());
		return false;
	}
	TestEqual(TEXT("Pipeline builds source"), Summary.Built, 1);

	TestTrue(
		TEXT("Collect built status"),
		FMASkillModuleBuildPipeline::CollectStatus(SourceDirectory, Items, Error));
	Item = Items.FindByPredicate([ModuleId](const FMASkillModuleBuildItem& Candidate)
	{
		return Candidate.ModuleId == ModuleId && !Candidate.SourceFile.IsEmpty();
	});
	TestNotNull(TEXT("Built status contains source module"), Item);
	if (!Item) return false;
	TestEqual(TEXT("Built source is up to date"), Item->Status, EMASkillModuleBuildStatus::Built);
	TestEqual(TEXT("Built source exposes its generated asset"), Item->GeneratedAssetPath, AssetPath);
	if (!TestTrue(
		TEXT("Rename built source to a noncanonical name"),
		IFileManager::Get().Move(*BuiltInvalidNameJsonFile, *JsonFile, false, false, false, true)))
	{
		return false;
	}
	TestTrue(
		TEXT("Collect status for renamed built source"),
		FMASkillModuleBuildPipeline::CollectStatus(SourceDirectory, Items, Error));
	TestFalse(
		TEXT("Content-resolved source does not orphan its generated asset"),
		Items.ContainsByPredicate([ModuleId](const FMASkillModuleBuildItem& Candidate)
		{
			return Candidate.ModuleId == ModuleId && Candidate.SourceFile.IsEmpty();
		}));
	if (!TestTrue(
		TEXT("Restore built source name"),
		IFileManager::Get().Move(*JsonFile, *BuiltInvalidNameJsonFile, false, false, false, true)))
	{
		return false;
	}

	const FString InvalidJson = FString::Printf(
		TEXT("{\"ModuleId\":%d,\"Module\":{}}"),
		ModuleId + 2);
	TestTrue(
		TEXT("Write invalid source JSON"),
		FFileHelper::SaveStringToFile(InvalidJson, *InvalidJsonFile));
	const TArray<FString> BuildFiles{InvalidJsonFile, JsonFile};
	TestFalse(
		TEXT("Batch reports individual build failures"),
		FMASkillModuleBuildPipeline::BuildFiles(
			SourceDirectory,
			BuildFiles,
			EMASkillModuleBuildMode::IfRequired,
			Summary,
			Error));
	TestEqual(TEXT("Batch continues after a failure"), Summary.UpToDate, 1);
	TestEqual(TEXT("Batch records failed source"), Summary.Failures.Num(), 1);
	IFileManager::Get().Delete(*InvalidJsonFile);

	TestTrue(TEXT("Delete source JSON"), IFileManager::Get().Delete(*JsonFile));
	if (!TestTrue(
		TEXT("Collect missing source status"),
		FMASkillModuleBuildPipeline::CollectStatus(SourceDirectory, Items, Error)))
	{
		AddError(Error.ToString());
		return false;
	}
	Item = Items.FindByPredicate([ModuleId](const FMASkillModuleBuildItem& Candidate)
	{
		return Candidate.ModuleId == ModuleId && Candidate.SourceFile.IsEmpty();
	});
	TestNotNull(TEXT("Missing source status contains generated module"), Item);
	if (!Item) return false;
	TestEqual(TEXT("Deleted JSON leaves an error"), Item->Status, EMASkillModuleBuildStatus::Error);
	TestEqual(TEXT("Missing source retains its generated asset"), Item->GeneratedAssetPath, AssetPath);

	AddExpectedError(
		TEXT("package was marked as deleted in editor"),
		EAutomationExpectedErrorFlags::Contains,
		0);
	if (!TestTrue(
		TEXT("Delete generated asset"),
		FMASkillModuleBuildPipeline::DeleteGeneratedAsset(AssetPath, Error)))
	{
		AddError(Error.ToString());
		return false;
	}
	TestFalse(TEXT("Delete removes generated package"), FPackageName::DoesPackageExist(PackageName));
	TestTrue(
		TEXT("Collect status after deleting generated asset"),
		FMASkillModuleBuildPipeline::CollectStatus(SourceDirectory, Items, Error));
	TestFalse(TEXT("Deleted orphan is removed from status"), Items.ContainsByPredicate([ModuleId](
		const FMASkillModuleBuildItem& Candidate)
	{
		return Candidate.ModuleId == ModuleId;
	}));
	return !HasAnyErrors();
}

#endif
