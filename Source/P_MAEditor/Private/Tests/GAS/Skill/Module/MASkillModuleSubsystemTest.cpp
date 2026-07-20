#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"
#include "Engine/GameInstance.h"
#include "GAS/Skill/Module/Build/MASkillModuleAssetBuilder.h"
#include "GAS/Skill/Module/MASkillModuleAsset.h"
#include "GAS/Skill/Module/Runtime/MASkillModuleSubsystem.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "PackageTools.h"
#include "UObject/Linker.h"
#include "UObject/StrongObjectPtr.h"

struct FMASkillModuleSubsystemTestContext
{
	~FMASkillModuleSubsystemTestContext()
	{
		UAssetManager& AssetManager = UAssetManager::Get();
		AssetManager.UnloadPrimaryAsset(AssetId);
		if (UObject* Asset = AssetPath.ResolveObject())
		{
			UPackage* Package = Asset->GetPackage();
			Package->SetDirtyFlag(false);
			IFileManager::Get().Delete(*PackageFilename, false, true, true);
			FAssetRegistryModule::AssetDeleted(Asset);
			ResetLoaders(Package);
		}
		else
		{
			IFileManager::Get().Delete(*PackageFilename, false, true, true);
		}
		AssetManager.RemoveScanPathsForPrimaryAssets(
			UMASkillModuleAsset::PrimaryAssetType,
			{GeneratedDirectory},
			UMASkillModuleAsset::StaticClass(),
			false);
		IFileManager::Get().DeleteDirectory(*GeneratedDiskDirectory, false, true);
		IFileManager::Get().DeleteDirectory(*SourceDirectory, false, true);
	}

	int32 ModuleId = 0;
	int32 CompletionCount = 0;
	double Deadline = 0.0;
	FString SourceDirectory;
	FString GeneratedDirectory;
	FString GeneratedDiskDirectory;
	FString PackageFilename;
	FPrimaryAssetId AssetId;
	FSoftObjectPath AssetPath;
	TStrongObjectPtr<UGameInstance> GameInstance;
	TStrongObjectPtr<UMASkillModuleSubsystem> ModuleSubsystem;
	const UMASkillModuleAsset* FirstResult = nullptr;
	const UMASkillModuleAsset* SecondResult = nullptr;
};

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FVerifySkillModuleRuntimeLoad,
	TSharedPtr<FMASkillModuleSubsystemTestContext>, Context,
	FAutomationTestBase*, Test);

bool FVerifySkillModuleRuntimeLoad::Update()
{
	if (Context->CompletionCount < 2 && FPlatformTime::Seconds() < Context->Deadline)
	{
		return false;
	}

	Test->TestEqual(TEXT("Both requests complete"), Context->CompletionCount, 2);
	Test->TestNotNull(TEXT("First request loads the generated asset"), Context->FirstResult);
	Test->TestTrue(
		TEXT("Duplicate requests resolve the same asset"),
		Context->FirstResult == Context->SecondResult);
	if (Context->FirstResult)
	{
		Test->TestEqual(
			TEXT("Loaded asset preserves ModuleId"),
			Context->FirstResult->GetModuleId(),
			Context->ModuleId);
		Test->TestEqual(
			TEXT("Loaded asset preserves module data"),
			Context->FirstResult->GetModuleData().ModuleName,
			FName(TEXT("RuntimeLoad")));
	}

	bool bInvalidRequestCompleted = false;
	Context->ModuleSubsystem->LoadModule(0, FMASkillModuleLoaded::CreateLambda(
		[&bInvalidRequestCompleted, this](const UMASkillModuleAsset* LoadedAsset)
		{
			bInvalidRequestCompleted = true;
			Test->TestNull(TEXT("Invalid ModuleId returns no asset"), LoadedAsset);
		}));
	Test->TestTrue(TEXT("Invalid ModuleId completes immediately"), bInvalidRequestCompleted);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMASkillModuleSubsystemTest,
	"P_MA.Skill.ModuleAsset.RuntimeLoad",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMASkillModuleSubsystemTest::RunTest(const FString& Parameters)
{
	const int32 ModuleId = 100000000 + static_cast<int32>(FGuid::NewGuid().A & 0x3fffffff);
	const FString TestId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	const FString SourceDirectory = FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("Automation/SkillModuleRuntimeLoad"),
		TestId);
	const FString GeneratedDirectory = TEXT("/Game/_Generated/SkillModules/__Automation/") + TestId;
	const FString JsonFile = FPaths::Combine(
		SourceDirectory,
		FString::Printf(TEXT("M_%d.json"), ModuleId));
	const FString PackageName = FMASkillModuleAssetBuilder::MakeAssetPackageName(GeneratedDirectory, ModuleId);
	const FString AssetName = FPaths::GetCleanFilename(PackageName);
	const FSoftObjectPath AssetPath(PackageName + TEXT(".") + AssetName);
	const FString PackageFilename =
		FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	const FString GeneratedDiskDirectory = FPackageName::LongPackageNameToFilename(GeneratedDirectory);
	const FPrimaryAssetId AssetId = UMASkillModuleAsset::MakePrimaryAssetId(ModuleId);
	UAssetManager& AssetManager = UAssetManager::Get();
	TSharedPtr<FMASkillModuleSubsystemTestContext> Context =
		MakeShared<FMASkillModuleSubsystemTestContext>();
	Context->ModuleId = ModuleId;
	Context->SourceDirectory = SourceDirectory;
	Context->GeneratedDirectory = GeneratedDirectory;
	Context->GeneratedDiskDirectory = GeneratedDiskDirectory;
	Context->PackageFilename = PackageFilename;
	Context->AssetId = AssetId;
	Context->AssetPath = AssetPath;

	if (!TestTrue(
		TEXT("Create source directory"),
		IFileManager::Get().MakeDirectory(*SourceDirectory, true)))
	{
		return false;
	}

	const FString Json = FString::Printf(
		TEXT("{\"ModuleId\":%d,\"Module\":{\"ModuleName\":\"RuntimeLoad\"}}"),
		ModuleId);
	if (!TestTrue(TEXT("Write source JSON"), FFileHelper::SaveStringToFile(Json, *JsonFile)))
	{
		return false;
	}

	EMASkillModuleAssetBuildResult BuildResult;
	FText Error;
	if (!TestTrue(
		TEXT("Build generated module asset"),
		FMASkillModuleAssetBuilder::Build(
			JsonFile,
			GeneratedDirectory,
			EMASkillModuleBuildMode::Force,
			BuildResult,
			Error)))
	{
		AddError(Error.ToString());
		return false;
	}

	AssetManager.ScanPathForPrimaryAssets(
		UMASkillModuleAsset::PrimaryAssetType,
		GeneratedDirectory,
		UMASkillModuleAsset::StaticClass(),
		false);

	UMASkillModuleAsset* BuiltAsset = Cast<UMASkillModuleAsset>(AssetPath.ResolveObject());
	TestNotNull(TEXT("Generated module asset exists before unload"), BuiltAsset);
	if (!BuiltAsset) return false;

	UPackage* BuiltPackage = BuiltAsset->GetPackage();
	BuiltPackage->SetDirtyFlag(false);
	FText UnloadError;
	if (!TestTrue(
		TEXT("Unload generated module asset before runtime load"),
		UPackageTools::UnloadPackages({BuiltPackage}, UnloadError, true)))
	{
		AddError(UnloadError.ToString());
		return false;
	}

	Context->GameInstance.Reset(NewObject<UGameInstance>());
	Context->ModuleSubsystem.Reset(NewObject<UMASkillModuleSubsystem>(Context->GameInstance.Get()));
	const TWeakPtr<FMASkillModuleSubsystemTestContext> WeakContext = Context;
	Context->ModuleSubsystem->LoadModule(ModuleId, FMASkillModuleLoaded::CreateLambda(
		[WeakContext](const UMASkillModuleAsset* LoadedAsset)
		{
			if (const TSharedPtr<FMASkillModuleSubsystemTestContext> PinnedContext = WeakContext.Pin())
			{
				PinnedContext->FirstResult = LoadedAsset;
				++PinnedContext->CompletionCount;
			}
		}));
	Context->ModuleSubsystem->LoadModule(ModuleId, FMASkillModuleLoaded::CreateLambda(
		[WeakContext](const UMASkillModuleAsset* LoadedAsset)
		{
			if (const TSharedPtr<FMASkillModuleSubsystemTestContext> PinnedContext = WeakContext.Pin())
			{
				PinnedContext->SecondResult = LoadedAsset;
				++PinnedContext->CompletionCount;
			}
		}));
	Context->Deadline = FPlatformTime::Seconds() + 10.0;
	ADD_LATENT_AUTOMATION_COMMAND(FVerifySkillModuleRuntimeLoad(Context, this));
	return true;
}

#endif
