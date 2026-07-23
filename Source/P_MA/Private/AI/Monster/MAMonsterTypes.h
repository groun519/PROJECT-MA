#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MAMonsterTypes.generated.h"

class UMASkillModule;
struct FMASkillModuleGroup;

USTRUCT(BlueprintType)
struct FMonsterSkillSlotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayTag SlotTag;

	UPROPERTY(EditAnywhere, meta=(ClampMin = "0.0"))
	float SelectionWeight = 1.f;

	UPROPERTY(EditAnywhere, meta=(ClampMin = "0.0"))
	float UseDistance = 300.f;

	UPROPERTY(EditAnywhere)
	TArray<TSoftObjectPtr<UMASkillModule>> Modules;
};

USTRUCT(BlueprintType)
struct FMonsterSkillPatternRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta=(ClampMin = "0.0"))
	float WindupDuration = 0.f;

	UPROPERTY(EditAnywhere, meta=(ClampMin = "0.0"))
	float UseDistance = 300.f;

	UPROPERTY(EditAnywhere)
	TArray<TSoftObjectPtr<UMASkillModule>> Modules;

	bool LoadModuleGroups(TArray<FMASkillModuleGroup>& OutModuleGroups) const;

#if WITH_EDITOR
	virtual void OnDataTableChanged(const UDataTable* InDataTable, FName InRowName) override;
#endif

private:
	// Generated from WindupDuration and serialized with the owning DataTable.
	UPROPERTY(Instanced)
	TObjectPtr<UMASkillModule> WindupSubModule = nullptr;
};
