#include "GAS/Skill/Module/MASkillModuleAsset.h"

const FPrimaryAssetType UMASkillModuleAsset::PrimaryAssetType(TEXT("SkillModule"));

FPrimaryAssetId UMASkillModuleAsset::GetPrimaryAssetId() const
{
	return ModuleId > 0
		? FPrimaryAssetId(PrimaryAssetType, FName(*LexToString(ModuleId)))
		: FPrimaryAssetId();
}

#if WITH_EDITOR
void UMASkillModuleAsset::SetGeneratedData(
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
