#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/Skill/Addon/MASkillModuleAddon.h"
#include "GAS/Skill/Module/MAModuleQualityData.h"
#include "GAS/Skill/Payload/MASkillPayloadEntry.h"
#include "GAS/Skill/Sequence/MASkillSequenceModifier.h"
#include "GAS/Skill/Sequence/MASkillSequenceTypes.h"
#include "GameplayTagContainer.h"
#include "MASkillDefinition.generated.h"

class UMASkillModuleStackAddon;
class UMASkillModuleInstance;
class UMASkillSequenceModifier;
class UTexture2D;
struct FMASkillModuleAddonRuntimeData;
struct FMASkillPayloadStore;
struct FMASkillAssembler;
struct FMASkillCooldownAssembler;
struct FMASkillPayloadAssembler;
struct FMASkillEventSourceAssembler;
struct FMASkillEventBindingAssembler;
struct FMASkillSequenceAssembler;

USTRUCT(BlueprintType)
struct FMASkillDefinitionIconData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Icon")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Icon")
	int32 Priority = 0;
};

struct FMASkillIconData
{
	UTexture2D* Icon = nullptr;
	FLinearColor IconColor = FLinearColor::White;
	FLinearColor InnerColor = FLinearColor(0.15f, 0.15f, 0.15f, 1.f);
};

USTRUCT(BlueprintType)
struct FMASkillDefinitionNameData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Name")
	FText Keyword;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Name")
	int32 Priority = 0;
};

USTRUCT(BlueprintType)
struct FMASkillDefinitionDisplayData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Display")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Display", meta=(MultiLine=true))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Display")
	FMASkillDefinitionIconData IconData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Display")
	FMASkillDefinitionNameData NameData;
};

UCLASS(BlueprintType)
class P_MA_API UMASkillDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	const FMASkillDefinitionDisplayData& GetDisplayData() const { return DisplayData; }
	const FMAModuleQuality& GetModuleQuality() const { return ModuleQuality; }
	FMASkillIconData ResolveIconData(const UMAModuleQualityData* ModuleQualityData) const;
	FLinearColor ResolveFrameColor(const UMAModuleQualityData* ModuleQualityData) const;
	UTexture2D* GetAssembledSubIcon() const { return AssembledSubIcon; }
	const FGameplayTagContainer& GetModuleTags() const { return ModuleTags; }
	FGameplayTagContainer GetTooltipTags() const;
	FGameplayTag GetVisualElementTag() const;
	const UMASkillModuleStackAddon* GetStackAddon() const;
	float GetCooldownSeconds() const;
	const TArray<FMASkillSequence>& GetBaseSequences() const { return BaseSequences; }
	const TArray<TObjectPtr<UMASkillSequenceModifier>>& GetSequenceModifiers() const { return SequenceModifiers; }
	const TArray<FMASkillSequence>& GetAssembledSequences() const { return AssembledSequences; }
	void InitializeAddonRuntimeData(FMASkillModuleAddonRuntimeData& RuntimeData) const;
	void ApplyAddonPayloadMirrors(const FMASkillModuleAddonRuntimeData& RuntimeData, FMASkillPayloadStore& PayloadStore) const;
	void BindAddons(UMASkillModuleInstance& ModuleInstance) const;
	void ForEachAddon(TFunctionRef<void(const UMASkillModuleAddon&)> Func) const;
	bool TryResolveSocketText(const FMASkillModuleAddonRuntimeData& RuntimeData, FText& OutText) const;
	virtual void PostLoad() override;

	template<typename AddonType>
	const AddonType* FindAddon() const
	{
		static_assert(TIsDerivedFrom<AddonType, UMASkillModuleAddon>::IsDerived,
			"AddonType must derive from UMASkillModuleAddon.");

		for (const TObjectPtr<UMASkillModuleAddon>& Addon : Addons)
		{
			if (const AddonType* TypedAddon = Cast<AddonType>(Addon.Get()))
			{
				return TypedAddon;
			}
		}
		return nullptr;
	}

	template<typename AddonType>
	bool HasAddon() const
	{
		return FindAddon<AddonType>() != nullptr;
	}

	template<typename ModifierType>
	ModifierType* AddTransientSequenceModifier()
	{
		static_assert(TIsDerivedFrom<ModifierType, UMASkillSequenceModifier>::IsDerived,
			"ModifierType must derive from UMASkillSequenceModifier.");
		check(HasAnyFlags(RF_Transient));

		ModifierType* Modifier = NewObject<ModifierType>(this, NAME_None, RF_Transient);
		if (Modifier)
		{
			SequenceModifiers.Add(Modifier);
		}
		return Modifier;
	}

	void ApplyPayloadsTo(FMASkillPayloadStore& PayloadStore) const
	{
		for (const FMASkillPayloadEntry& PayloadEntry : Payloads)
		{
			PayloadEntry.ApplyTo(PayloadStore);
		}
	}

private:
	void ResetAssemblyData();

	friend struct FMASkillAssembler;
	friend struct FMASkillCooldownAssembler;
	friend struct FMASkillPayloadAssembler;
	friend struct FMASkillEventSourceAssembler;
	friend struct FMASkillEventBindingAssembler;
	friend struct FMASkillSequenceAssembler;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Display", meta=(AllowPrivateAccess="true"))
	FMASkillDefinitionDisplayData DisplayData;

	UPROPERTY(EditDefaultsOnly, Category="Module")
	FMAModuleQuality ModuleQuality;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> AssembledSubIcon = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Tags", meta=(Categories="Module"))
	FGameplayTagContainer ModuleTags;

	UPROPERTY(EditDefaultsOnly, Category="Visual", meta=(Categories="Module.Visual"))
	FGameplayTagContainer ModuleVisualTags;

	UPROPERTY()
	FGameplayTag ExclusiveAssemblyTag_DEPRECATED;

	UPROPERTY()
	FGameplayTag UniqueModuleEffectTag_DEPRECATED;

	UPROPERTY(EditDefaultsOnly, Category="Sequence")
	TArray<FMASkillSequence> BaseSequences;

	UPROPERTY(EditDefaultsOnly, Instanced, Category="Sequence")
	TArray<TObjectPtr<UMASkillSequenceModifier>> SequenceModifiers;

	UPROPERTY(Transient)
	TArray<FMASkillSequence> AssembledSequences;

	UPROPERTY(EditDefaultsOnly, Instanced, Category="Addon")
	TArray<TObjectPtr<UMASkillModuleAddon>> Addons;

	UPROPERTY(EditDefaultsOnly, Category="Payload")
	TArray<FMASkillPayloadEntry> Payloads;

};

