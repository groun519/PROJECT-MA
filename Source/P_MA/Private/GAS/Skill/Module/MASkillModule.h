#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/Skill/Module/MASkillModuleDataTypes.h"
#include "MASkillModule.generated.h"

class UMAModuleQualityData;
class UMASkillModuleInstance;
class UMASkillModuleStackAddon;
struct FMASkillModuleAddonRuntimeData;
struct FMASkillPayloadStore;
struct FMASkillAssembler;
struct FMASkillModuleAssembler;
struct FMASkillCooldownAssembler;
struct FMASkillPayloadAssembler;
struct FMASkillEventSourceAssembler;
struct FMASkillEventBindingAssembler;
struct FMASkillSequenceAssembler;

/** Skill module data object used by generated sources and transient assembly results. */
UCLASS(NotBlueprintable)
class P_MA_API UMASkillModule : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const FPrimaryAssetType PrimaryAssetType;
	static FPrimaryAssetId MakePrimaryAssetId(int32 ModuleId);
	static FName GetModuleIdTag() { return GET_MEMBER_NAME_CHECKED(UMASkillModule, ModuleId); }

#if WITH_EDITOR
	static constexpr int32 CurrentGeneratedDataVersion = 1;
	static FName GetSourceHashTag() { return GET_MEMBER_NAME_CHECKED(UMASkillModule, SourceHash); }
	static FName GetGeneratedDataVersionTag()
	{
		return GET_MEMBER_NAME_CHECKED(UMASkillModule, GeneratedDataVersion);
	}
	static FName GetLastBuiltAtTag() { return GET_MEMBER_NAME_CHECKED(UMASkillModule, LastBuiltAt); }
#endif

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	int32 GetModuleId() const { return ModuleId; }
	const FMASkillModuleData& GetModuleData() const { return ModuleData; }
	const FMASkillDefinitionDisplayData& GetDisplayData() const { return ModuleData.DisplayData; }
	const FMAModuleQuality& GetModuleQuality() const { return ModuleData.ModuleQuality; }
	FMASkillIconData ResolveIconData(const UMAModuleQualityData* ModuleQualityData) const;
	FLinearColor ResolveFrameColor(const UMAModuleQualityData* ModuleQualityData) const;
	UTexture2D* GetAssembledSubIcon() const { return ModuleData.AssembledSubIcon; }
	const FGameplayTagContainer& GetModuleTags() const { return ModuleData.ModuleTags; }
	FGameplayTagContainer GetTooltipTags() const { return ModuleData.ModuleTags; }
	FGameplayTag GetVisualElementTag() const;
	const UMASkillModuleStackAddon* GetStackAddon() const;
	float GetCooldownSeconds() const;
	void InitializeAddonRuntimeData(FMASkillModuleAddonRuntimeData& RuntimeData) const;
	void ApplyAddonPayloadMirrors(
		const FMASkillModuleAddonRuntimeData& RuntimeData,
		FMASkillPayloadStore& PayloadStore) const;
	void BindAddons(UMASkillModuleInstance& ModuleInstance) const;
	void ForEachAddon(TFunctionRef<void(const UMASkillModuleAddon&)> Func) const;
	bool TryResolveSocketText(
		const FMASkillModuleAddonRuntimeData& RuntimeData,
		FText& OutText) const;

	template<typename AddonType>
	const AddonType* FindAddon() const
	{
		static_assert(TIsDerivedFrom<AddonType, UMASkillModuleAddon>::IsDerived,
			"AddonType must derive from UMASkillModuleAddon.");

		for (const TObjectPtr<UMASkillModuleAddon>& Addon : ModuleData.Addons)
		{
			if (const AddonType* TypedAddon = Cast<AddonType>(Addon.Get()))
			{
				return TypedAddon;
			}
		}
		return nullptr;
	}

	template<typename AddonType>
	AddonType* FindMutableAddon()
	{
		static_assert(TIsDerivedFrom<AddonType, UMASkillModuleAddon>::IsDerived,
			"AddonType must derive from UMASkillModuleAddon.");

		for (const TObjectPtr<UMASkillModuleAddon>& Addon : ModuleData.Addons)
		{
			if (AddonType* TypedAddon = Cast<AddonType>(Addon.Get()))
			{
				return TypedAddon;
			}
		}
		return nullptr;
	}

	void ApplyPayloadsTo(FMASkillPayloadStore& PayloadStore) const;

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
	void ResetAssemblyData();

	friend struct FMASkillAssembler;
	friend struct FMASkillModuleAssembler;
	friend struct FMASkillCooldownAssembler;
	friend struct FMASkillPayloadAssembler;
	friend struct FMASkillEventSourceAssembler;
	friend struct FMASkillEventBindingAssembler;
	friend struct FMASkillSequenceAssembler;

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
