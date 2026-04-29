#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MASkillGenericDataAsset.generated.h"

class UDataTable;

UCLASS(BlueprintType)
class P_MA_API UMASkillGenericDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	const UDataTable* GetElementalDataTable() const { return ElementalDataTable; }
	const UDataTable* GetOverlapDecalDataTable() const { return OverlapDecalDataTable; }

private:
	UPROPERTY(EditDefaultsOnly, Category="Elemental", meta=(RowType="/Script/P_MA.MAElementDataRow"))
	TObjectPtr<UDataTable> ElementalDataTable;

	UPROPERTY(EditDefaultsOnly, Category="Effect", meta=(RowType="/Script/P_MA.MAOverlapDecalDataRow"))
	TObjectPtr<UDataTable> OverlapDecalDataTable;
};
