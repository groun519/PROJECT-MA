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
	int32& OutModuleId,
	FName& OutModuleName,
	EMAModuleRarity& OutModuleRarity,
	FText& OutError)
{
	TArray<uint8> Bytes;
	if (!LoadBytes(FilePath, Bytes, OutError)) return false;

	FString Json;
	FFileHelper::BufferToString(Json, Bytes.GetData(), Bytes.Num());
	return FMASkillModuleJsonReader::ReadHeader(
		Json,
		OutModuleId,
		OutModuleName,
		OutModuleRarity,
		OutError);
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
	FName IgnoredModuleName;
	EMAModuleRarity IgnoredModuleRarity;
	return ReadHeader(SourceFile, OutModuleId, IgnoredModuleName, IgnoredModuleRarity, OutError);
}
