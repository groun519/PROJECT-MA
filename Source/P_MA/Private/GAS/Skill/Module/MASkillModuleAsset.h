#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/Skill/Module/MASkillModuleDataTypes.h"
#include "MASkillModuleAsset.generated.h"

/** Generated runtime asset built from one skill module JSON file. */
UCLASS(NotBlueprintable)
class P_MA_API UMASkillModuleAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const FPrimaryAssetType PrimaryAssetType;
	static FName GetModuleIdTag() { return GET_MEMBER_NAME_CHECKED(UMASkillModuleAsset, ModuleId); }

#if WITH_EDITOR
	static constexpr int32 CurrentGeneratedDataVersion = 1;
	static FName GetSourceHashTag() { return GET_MEMBER_NAME_CHECKED(UMASkillModuleAsset, SourceHash); }
	static FName GetGeneratedDataVersionTag()
	{
		return GET_MEMBER_NAME_CHECKED(UMASkillModuleAsset, GeneratedDataVersion);
	}
	static FName GetLastBuiltAtTag() { return GET_MEMBER_NAME_CHECKED(UMASkillModuleAsset, LastBuiltAt); }
#endif

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	int32 GetModuleId() const { return ModuleId; }
	const FMASkillModuleData& GetModuleData() const { return ModuleData; }

#if WITH_EDITOR
	const FString& GetSourceHash() const { return SourceHash; }
	int32 GetGeneratedDataVersion() const { return GeneratedDataVersion; }
	int64 GetLastBuiltAt() const { return LastBuiltAt; }

	void SetGeneratedData(
		int32 InModuleId,
		FMASkillModuleData&& InModuleData,
		FString&& InSourceHash,
		int32 InGeneratedDataVersion = CurrentGeneratedDataVersion,
		int64 InLastBuiltAt = -1);
#endif

private:
	UPROPERTY(VisibleAnywhere, AssetRegistrySearchable, Category="Module")
	int32 ModuleId = 0;

	UPROPERTY(VisibleAnywhere, Category="Module")
	FMASkillModuleData ModuleData;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, AssetRegistrySearchable, Category="Build")
	FString SourceHash;

	UPROPERTY(VisibleAnywhere, AssetRegistrySearchable, Category="Build")
	int32 GeneratedDataVersion = 0;

	UPROPERTY(VisibleAnywhere, AssetRegistrySearchable, Category="Build")
	int64 LastBuiltAt = 0;
#endif
};
