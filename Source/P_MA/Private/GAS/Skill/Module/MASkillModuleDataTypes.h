#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Addon/MASkillModuleAddon.h"
#include "GAS/Skill/Module/MAModuleQualityData.h"
#include "GAS/Skill/Payload/MASkillPayloadEntry.h"
#include "GameplayTagContainer.h"
#include "MASkillModuleDataTypes.generated.h"

class UTexture2D;

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

/** Authored module data shared by source modules and assembled runtime results. */
USTRUCT(BlueprintType)
struct P_MA_API FMASkillModuleData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Module")
	EMASkillModuleType ModuleType = EMASkillModuleType::Module;

	UPROPERTY(EditAnywhere, Category="Module")
	FName ModuleName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Display")
	FMASkillDefinitionDisplayData DisplayData;

	UPROPERTY(EditAnywhere, Category="Module")
	FMAModuleQuality ModuleQuality;

	UPROPERTY(EditAnywhere, Category="Tags", meta=(Categories="Module"))
	FGameplayTagContainer ModuleTags;

	UPROPERTY(EditAnywhere, Category="Visual", meta=(Categories="Module.Visual"))
	FGameplayTagContainer ModuleVisualTags;

	UPROPERTY(EditAnywhere, Instanced, Category="Addon")
	TArray<TObjectPtr<UMASkillModuleAddon>> Addons;

	UPROPERTY(EditAnywhere, Category="Payload")
	TArray<FMASkillPayloadEntry> Payloads;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> AssembledSubIcon = nullptr;
};
