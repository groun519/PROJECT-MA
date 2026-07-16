#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "GAS/Skill/Addon/Effect/MASkillModuleGameplayEffectAddon.h"
#include "GAS/Skill/Module/MASkillModuleAsset.h"
#include "GAS/Skill/Module/Build/MASkillModuleAssetBuilder.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "UObject/Linker.h"
#include "UObject/PackageReload.h"
#include "UObject/SavePackage.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMASkillModuleAssetBuilderTest,
	"P_MA.Skill.ModuleAsset.Build",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMASkillModuleAssetBuilderTest::RunTest(const FString& Parameters)
{
	const FString TestId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	const FString SourceDirectory = FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("Automation/SkillModuleBuild"),
		TestId);
	const FString GeneratedDirectory = TEXT("/Game/__Automation/SkillModuleBuild/") + TestId;
	const FString JsonFile = FPaths::Combine(SourceDirectory, TEXT("M_1201.json"));
	const FString PackageName = FMASkillModuleAssetBuilder::MakeAssetPackageName(GeneratedDirectory, 1201);
	const FString AssetName = FPaths::GetCleanFilename(PackageName);
	const FSoftObjectPath AssetPath(PackageName + TEXT(".") + AssetName);
	const FString PackageFilename =
		FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	const FString GeneratedDiskDirectory = FPackageName::LongPackageNameToFilename(GeneratedDirectory);

	ON_SCOPE_EXIT
	{
		if (UObject* Asset = AssetPath.ResolveObject())
		{
			Asset->GetPackage()->SetDirtyFlag(false);
			ResetLoaders(Asset->GetPackage());
		}
		IFileManager::Get().Delete(*PackageFilename, false, true, true);
		IFileManager::Get().DeleteDirectory(*GeneratedDiskDirectory, false, true);
		IFileManager::Get().DeleteDirectory(*SourceDirectory, false, true);
	};

	if (!TestTrue(
		TEXT("Create source directory"),
		IFileManager::Get().MakeDirectory(*SourceDirectory, true)))
	{
		return false;
	}

	const FString InitialJson = TEXT(
		"{\"ModuleId\":1201,\"Module\":{\"ModuleName\":\"First\",\"Addons\":["
		"{\"_ClassName\":\"/Script/P_MA.MASkillModuleGameplayEffectAddon\","
		"\"GameplayEffects\":[{}]}]}}");
	if (!TestTrue(
		TEXT("Write source JSON"),
		FFileHelper::SaveStringToFile(InitialJson, *JsonFile)))
	{
		return false;
	}

	FText Error;
	EMASkillModuleAssetBuildResult Result;
	FMASkillModuleBuildItem BuildItem;
	if (!TestTrue(
		TEXT("Inspect source before first build"),
		FMASkillModuleAssetBuilder::ResolveBuildStatus(JsonFile, GeneratedDirectory, BuildItem, Error)))
	{
		AddError(Error.ToString());
		return false;
	}
	TestEqual(TEXT("Source without generated asset requires build"), BuildItem.Status, EMASkillModuleBuildStatus::NeedsBuild);

	if (!TestTrue(
		TEXT("Build generated module asset"),
		FMASkillModuleAssetBuilder::Build(
			JsonFile,
			GeneratedDirectory,
			EMASkillModuleBuildMode::IfRequired,
			Result,
			Error)))
	{
		AddError(Error.ToString());
		return false;
	}
	TestEqual(TEXT("First build writes the asset"), Result, EMASkillModuleAssetBuildResult::Built);

	UMASkillModuleAsset* BuiltAsset = LoadObject<UMASkillModuleAsset>(nullptr, *AssetPath.ToString());
	TestNotNull(TEXT("Generated module asset exists"), BuiltAsset);
	if (!BuiltAsset) return false;
	TestEqual(TEXT("Generated asset preserves ModuleId"), BuiltAsset->GetModuleId(), 1201);
	TestEqual(TEXT("Generated asset preserves module data"), BuiltAsset->GetModuleData().ModuleName, FName(TEXT("First")));
	TestEqual(
		TEXT("Generated asset exposes its Primary Asset id"),
		BuiltAsset->GetPrimaryAssetId(),
		FPrimaryAssetId(UMASkillModuleAsset::PrimaryAssetType, TEXT("1201")));
	TestFalse(TEXT("Generated asset records its source hash"), BuiltAsset->GetSourceHash().IsEmpty());
	TestEqual(
		TEXT("Generated asset records its data version"),
		BuiltAsset->GetGeneratedDataVersion(),
		UMASkillModuleAsset::CurrentGeneratedDataVersion);
	TestTrue(TEXT("Generated asset records its build time"), BuiltAsset->GetLastBuiltAt() > 0);
	const int64 InitialLastBuiltAt = BuiltAsset->GetLastBuiltAt();
	if (!BuiltAsset->GetModuleData().Addons.IsEmpty())
	{
		TestTrue(
			TEXT("Generated addon is owned by its module asset"),
			BuiltAsset->GetModuleData().Addons[0]->GetOuter() == BuiltAsset);
	}
	if (!TestTrue(
		TEXT("Inspect built source"),
		FMASkillModuleAssetBuilder::ResolveBuildStatus(JsonFile, GeneratedDirectory, BuildItem, Error)))
	{
		AddError(Error.ToString());
		return false;
	}
	TestEqual(TEXT("Built source is up to date"), BuildItem.Status, EMASkillModuleBuildStatus::Built);
	TestEqual(TEXT("Build status exposes the build time"), BuildItem.LastBuiltAt, InitialLastBuiltAt);

	if (!TestTrue(
		TEXT("Unchanged source skips rebuilding"),
		FMASkillModuleAssetBuilder::Build(
			JsonFile,
			GeneratedDirectory,
			EMASkillModuleBuildMode::IfRequired,
			Result,
			Error)))
	{
		AddError(Error.ToString());
		return false;
	}
	TestEqual(TEXT("Unchanged source is up to date"), Result, EMASkillModuleAssetBuildResult::UpToDate);
	TestEqual(TEXT("Skipped build preserves the build time"), BuiltAsset->GetLastBuiltAt(), InitialLastBuiltAt);
	if (!TestTrue(
		TEXT("Force rebuild unchanged source"),
		FMASkillModuleAssetBuilder::Build(
			JsonFile,
			GeneratedDirectory,
			EMASkillModuleBuildMode::Force,
			Result,
			Error)))
	{
		AddError(Error.ToString());
		return false;
	}
	TestEqual(TEXT("Force mode rebuilds unchanged source"), Result, EMASkillModuleAssetBuildResult::Built);

	FMASkillModuleData PreviousVersionData = BuiltAsset->GetModuleData();
	BuiltAsset->SetGeneratedData(
		BuiltAsset->GetModuleId(),
		MoveTemp(PreviousVersionData),
		FString(BuiltAsset->GetSourceHash()),
		UMASkillModuleAsset::CurrentGeneratedDataVersion - 1);
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;
	if (!TestTrue(
		TEXT("Save previous generated data version"),
		UPackage::SavePackage(BuiltAsset->GetPackage(), BuiltAsset, *PackageFilename, SaveArgs)))
	{
		return false;
	}
	TArray<FAssetData> SavedAssets;
	SavedAssets.Emplace(BuiltAsset);
	FAssetRegistryModule::AssetsSaved(MoveTemp(SavedAssets));
	if (!TestTrue(
		TEXT("Inspect previous generated data version"),
		FMASkillModuleAssetBuilder::ResolveBuildStatus(JsonFile, GeneratedDirectory, BuildItem, Error)))
	{
		AddError(Error.ToString());
		return false;
	}
	TestEqual(
		TEXT("Previous generated data version requires rebuild"),
		BuildItem.Status,
		EMASkillModuleBuildStatus::NeedsBuild);
	if (!TestTrue(
		TEXT("Rebuild previous generated data version"),
		FMASkillModuleAssetBuilder::Build(
			JsonFile,
			GeneratedDirectory,
			EMASkillModuleBuildMode::IfRequired,
			Result,
			Error)))
	{
		AddError(Error.ToString());
		return false;
	}
	TestEqual(TEXT("Previous version is rebuilt"), Result, EMASkillModuleAssetBuildResult::Built);

	const FString UpdatedJson = TEXT(
		"{\"ModuleId\":1201,\"Module\":{\"ModuleName\":\"Updated\",\"Addons\":["
		"{\"_ClassName\":\"/Script/P_MA.MASkillModuleGameplayEffectAddon\","
		"\"GameplayEffects\":[{}]}]}}");
	TestTrue(TEXT("Update source JSON"), FFileHelper::SaveStringToFile(UpdatedJson, *JsonFile));
	if (!TestTrue(
		TEXT("Inspect changed source"),
		FMASkillModuleAssetBuilder::ResolveBuildStatus(JsonFile, GeneratedDirectory, BuildItem, Error)))
	{
		AddError(Error.ToString());
		return false;
	}
	TestEqual(TEXT("Changed source requires rebuild"), BuildItem.Status, EMASkillModuleBuildStatus::NeedsBuild);
	if (!TestTrue(
		TEXT("Changed source rebuilds the asset"),
		FMASkillModuleAssetBuilder::Build(
			JsonFile,
			GeneratedDirectory,
			EMASkillModuleBuildMode::IfRequired,
			Result,
			Error)))
	{
		AddError(Error.ToString());
		return false;
	}
	TestEqual(TEXT("Changed source writes the asset"), Result, EMASkillModuleAssetBuildResult::Built);
	TestEqual(
		TEXT("Rebuild replaces module data"),
		BuiltAsset->GetModuleData().ModuleName,
		FName(TEXT("Updated")));

	const FString MismatchedJsonFile = FPaths::Combine(SourceDirectory, TEXT("M_1202.json"));
	TestTrue(
		TEXT("Write mismatched source JSON"),
		FFileHelper::SaveStringToFile(TEXT("{\"ModuleId\":1203,\"Module\":{}}"), *MismatchedJsonFile));
	TestTrue(
		TEXT("Inspect mismatched source JSON"),
		FMASkillModuleAssetBuilder::ResolveBuildStatus(
			MismatchedJsonFile,
			GeneratedDirectory,
			BuildItem,
			Error));
	TestEqual(
		TEXT("Mismatched source is reported as an error during refresh"),
		BuildItem.Status,
		EMASkillModuleBuildStatus::Error);
	TestFalse(TEXT("Mismatched source describes its error"), BuildItem.StatusDetail.IsEmpty());
	TestFalse(
		TEXT("ModuleId must match the source filename"),
		FMASkillModuleAssetBuilder::Build(
			MismatchedJsonFile,
			GeneratedDirectory,
			EMASkillModuleBuildMode::IfRequired,
			Result,
			Error));

	const FString InvalidSchemaJsonFile = FPaths::Combine(SourceDirectory, TEXT("M_1204.json"));
	TestTrue(
		TEXT("Write schema-invalid source JSON"),
		FFileHelper::SaveStringToFile(
			TEXT("{\"ModuleId\":1204,\"Module\":{\"UnknownField\":1}}"),
			*InvalidSchemaJsonFile));
	TestTrue(
		TEXT("Inspect schema-invalid source JSON"),
		FMASkillModuleAssetBuilder::ResolveBuildStatus(
			InvalidSchemaJsonFile,
			GeneratedDirectory,
			BuildItem,
			Error));
	TestEqual(
		TEXT("Schema-invalid source is reported as an error during refresh"),
		BuildItem.Status,
		EMASkillModuleBuildStatus::Error);
	TestTrue(
		TEXT("Schema-invalid source identifies the invalid field"),
		BuildItem.StatusDetail.ToString().Contains(TEXT("Module.UnknownField")));

	UPackage* ReloadedPackage = ReloadPackage(BuiltAsset->GetPackage(), LOAD_None);
	TestNotNull(TEXT("Generated package reloads"), ReloadedPackage);
	if (!ReloadedPackage) return false;

	UMASkillModuleAsset* ReloadedAsset = FindObject<UMASkillModuleAsset>(ReloadedPackage, *AssetName);
	TestNotNull(TEXT("Generated asset survives package reload"), ReloadedAsset);
	if (!ReloadedAsset || ReloadedAsset->GetModuleData().Addons.IsEmpty()) return false;

	const UMASkillModuleGameplayEffectAddon* EffectAddon =
		Cast<UMASkillModuleGameplayEffectAddon>(ReloadedAsset->GetModuleData().Addons[0]);
	TestNotNull(TEXT("Generated effect addon survives package reload"), EffectAddon);
	if (!EffectAddon) return false;

	const FArrayProperty* GameplayEffectsProperty =
		FindFProperty<FArrayProperty>(EffectAddon->GetClass(), TEXT("GameplayEffects"));
	TestNotNull(TEXT("Effect addon exposes generated effect configs"), GameplayEffectsProperty);
	if (!GameplayEffectsProperty) return false;

	FScriptArrayHelper GameplayEffects(
		GameplayEffectsProperty,
		GameplayEffectsProperty->ContainerPtrToValuePtr<void>(
			const_cast<UMASkillModuleGameplayEffectAddon*>(EffectAddon)));
	TestEqual(TEXT("Effect addon preserves its generated config"), GameplayEffects.Num(), 1);
	if (GameplayEffects.Num() == 0) return false;

	const FMASkillModuleGameplayEffectConfig* EffectConfig =
		reinterpret_cast<const FMASkillModuleGameplayEffectConfig*>(GameplayEffects.GetRawPtr(0));
	TestNotNull(TEXT("Generated effect definition survives package reload"), EffectConfig->GetEffectDefinition());
	return !HasAnyErrors();
}

#endif
