#include "GAS/Skill/Module/Json/MASkillModuleJsonFile.h"

#include "GAS/Skill/Module/Json/MASkillModuleJsonReader.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"

static bool Fail(FText& OutError, const FString& Message)
{
	OutError = FText::FromString(Message);
	return false;
}

FString FMASkillModuleJsonSource::ToJson() const
{
	FString Json;
	FFileHelper::BufferToString(Json, Bytes.GetData(), Bytes.Num());
	return Json;
}

static bool LoadBytes(
	const FString& FilePath,
	TArray<uint8>& OutBytes,
	FText& OutError)
{
	if (!FFileHelper::LoadFileToArray(OutBytes, *FilePath))
	{
		return Fail(OutError, FString::Printf(TEXT("Failed to read module JSON: %s"), *FilePath));
	}
	return true;
}

bool FMASkillModuleJsonFile::Load(
	const FString& FilePath,
	FMASkillModuleJsonSource& OutSource,
	FText& OutError)
{
	OutSource = FMASkillModuleJsonSource();
	OutError = FText::GetEmpty();
	if (!ResolveModuleId(FilePath, OutSource.ModuleId, OutError)) return false;
	if (!LoadBytes(FilePath, OutSource.Bytes, OutError)) return false;

	FMD5 Hash;
	Hash.Update(OutSource.Bytes.GetData(), OutSource.Bytes.Num());
	uint8 Digest[16];
	Hash.Final(Digest);
	OutSource.SourceHash = BytesToHex(Digest, UE_ARRAY_COUNT(Digest));
	return true;
}

bool FMASkillModuleJsonFile::Save(
	const FString& FilePath,
	const FString& Json,
	const bool bReplaceExisting,
	FText& OutError)
{
	const FString Directory = FPaths::GetPath(FilePath);
	if (!IFileManager::Get().DirectoryExists(*Directory)
		&& !IFileManager::Get().MakeDirectory(*Directory, true))
	{
		return Fail(OutError, FString::Printf(TEXT("Failed to create module JSON directory: %s"), *Directory));
	}
	if (!bReplaceExisting && IFileManager::Get().FileExists(*FilePath))
	{
		return Fail(OutError, FString::Printf(TEXT("Module JSON already exists: %s"), *FilePath));
	}

	const FString TemporaryFile = FPaths::CreateTempFilename(
		*FPaths::GetPath(FilePath),
		TEXT("SkillModule_"),
		TEXT(".tmp"));
	if (!FFileHelper::SaveStringToFile(
		Json,
		*TemporaryFile,
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		IFileManager::Get().Delete(*TemporaryFile);
		return Fail(OutError, FString::Printf(TEXT("Failed to write temporary module JSON: %s"), *TemporaryFile));
	}
	if (!IFileManager::Get().Move(*FilePath, *TemporaryFile, bReplaceExisting, false, false, true))
	{
		IFileManager::Get().Delete(*TemporaryFile);
		return Fail(OutError, FString::Printf(TEXT("Failed to save module JSON: %s"), *FilePath));
	}
	return true;
}

bool FMASkillModuleJsonFile::ReadHeader(
	const FString& FilePath,
	FMASkillModuleJsonHeader& OutHeader,
	FText& OutError)
{
	TArray<uint8> Bytes;
	if (!LoadBytes(FilePath, Bytes, OutError)) return false;

	FString Json;
	FFileHelper::BufferToString(Json, Bytes.GetData(), Bytes.Num());
	return FMASkillModuleJsonReader::ReadHeader(
		Json,
		OutHeader,
		OutError);
}

static FString GetTypeFolderName(const EMASkillModuleType ModuleType)
{
	switch (ModuleType)
	{
	case EMASkillModuleType::Module:
		return TEXT("Module");
	case EMASkillModuleType::Item:
		return TEXT("Item");
	case EMASkillModuleType::Sub:
		return TEXT("Sub");
	default:
		return FString();
	}
}

FString FMASkillModuleJsonFile::MakeSourceFilePath(
	const FString& SourceDirectory,
	const int32 ModuleId,
	const EMASkillModuleType ModuleType)
{
	const FString TypeFolder = GetTypeFolderName(ModuleType);
	return ModuleId > 0 && !TypeFolder.IsEmpty()
		? FPaths::Combine(
			SourceDirectory,
			TypeFolder,
			FString::Printf(TEXT("M_%d.json"), ModuleId))
		: FString();
}

bool FMASkillModuleJsonFile::ValidateSourceFilePath(
	const FString& SourceDirectory,
	const FString& SourceFile,
	const EMASkillModuleType ModuleType,
	FText& OutError)
{
	const FString TypeFolder = GetTypeFolderName(ModuleType);
	if (TypeFolder.IsEmpty())
	{
		return Fail(OutError, TEXT("Skill module source has an invalid ModuleType."));
	}

	FString ExpectedDirectory = FPaths::ConvertRelativePathToFull(
		FPaths::Combine(SourceDirectory, TypeFolder));
	FPaths::NormalizeDirectoryName(ExpectedDirectory);

	FString ActualDirectory = FPaths::ConvertRelativePathToFull(FPaths::GetPath(SourceFile));
	FPaths::NormalizeDirectoryName(ActualDirectory);
	if (FPaths::IsSamePath(ActualDirectory, ExpectedDirectory))
	{
		OutError = FText::GetEmpty();
		return true;
	}

	return Fail(OutError, FString::Printf(
		TEXT("%s module JSON must be stored under: %s"),
		*TypeFolder,
		*ExpectedDirectory));
}

bool FMASkillModuleJsonFile::ResolveModuleId(
	const FString& SourceFile,
	int32& OutModuleId,
	FText& OutError)
{
	OutModuleId = 0;
	const FString FileName = FPaths::GetCleanFilename(SourceFile);
	FString IdText = FPaths::GetBaseFilename(FileName);
	if (!FileName.EndsWith(TEXT(".json"), ESearchCase::IgnoreCase)
		|| !IdText.RemoveFromStart(TEXT("M_"), ESearchCase::CaseSensitive)
		|| !LexTryParseString(OutModuleId, *IdText)
		|| OutModuleId <= 0
		|| IdText != LexToString(OutModuleId))
	{
		return Fail(OutError, FString::Printf(
			TEXT("Skill module source file must be named M_<positive ModuleId>.json: %s"),
			*FileName));
	}

	return true;
}

bool FMASkillModuleJsonFile::ResolveModuleIdFromContent(
	const FString& SourceFile,
	int32& OutModuleId,
	FText& OutError)
{
	FMASkillModuleJsonHeader Header;
	if (!ReadHeader(SourceFile, Header, OutError)) return false;
	OutModuleId = Header.ModuleId;
	return true;
}
