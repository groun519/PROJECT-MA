#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/Skill/Event/Binding/MASkillGameplayEventBinding.h"
#include "GAS/Skill/Module/MAModuleQualityData.h"
#include "GAS/Skill/Payload/MASkillPayloadEntry.h"
#include "GAS/Skill/Step/MASkillStep.h"
#include "GameplayTagContainer.h"
#include "MASkillDefinition.generated.h"

class UMASkillEventSource;
class UMASkillModuleInstance;
class UTexture2D;
struct FMASkillPayloadStore;
struct FMASkillAssembler;

USTRUCT(BlueprintType)
struct FMASkillDefinitionIconData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Icon")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Icon")
	FLinearColor IconColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Icon")
	FLinearColor InnerColor = FLinearColor(0.15f, 0.15f, 0.15f, 1.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Icon")
	int32 Priority = 0;
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
	UTexture2D* GetAssembledSubIcon() const { return AssembledSubIcon; }
	const FGameplayTag& GetExclusiveAssemblyTag() const { return ExclusiveAssemblyTag; }
	const FGameplayTag& GetElementalTag() const { return ElementalTag; }
	const TArray<TObjectPtr<UMASkillStep>>& GetSkillSteps() const { return SkillSteps; }
	const TArray<FMASkillGameplayEventBinding>& GetEventBindings() const { return EventBindings; }
	const TArray<TObjectPtr<UMASkillEventSource>>& GetEventSources() const { return EventSources; }

	void ApplyPayloadsTo(FMASkillPayloadStore& PayloadStore) const
	{
		for (const FMASkillPayloadEntry& PayloadEntry : Payloads)
		{
			PayloadEntry.ApplyTo(PayloadStore);
		}
	}

private:
	void ResetAssemblyData();
	void AppendFrom(UMASkillModuleInstance* SourceModuleInstance);
	void FinalizeStepAssembly();

	friend struct FMASkillAssembler;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Display", meta=(AllowPrivateAccess="true"))
	FMASkillDefinitionDisplayData DisplayData;

	UPROPERTY(EditDefaultsOnly, Category="Module")
	FMAModuleQuality ModuleQuality;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> AssembledSubIcon = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Assembly", meta=(Categories="Module.Assembly.Exclusive"))
	FGameplayTag ExclusiveAssemblyTag;

	UPROPERTY(EditDefaultsOnly, Category="Elemental", meta=(Categories="Elemental"))
	FGameplayTag ElementalTag;

	/** Preferred step pipeline. Each step owns its own montage and runtime logic. **/
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Step")
	TArray<TObjectPtr<UMASkillStep>> SkillSteps;

	/** Event Source **/
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Event")
	TArray<TObjectPtr<UMASkillEventSource>> EventSources;

	/** Event **/
	UPROPERTY(EditDefaultsOnly, Category="Event", meta=(DisplayName="Event Bindings"))
	TArray<FMASkillGameplayEventBinding> EventBindings;

	UPROPERTY(EditDefaultsOnly, Category="Payload")
	TArray<FMASkillPayloadEntry> Payloads;

};
