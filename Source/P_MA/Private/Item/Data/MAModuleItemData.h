#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MAModuleItemData.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct P_MA_API FMAModuleItemDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Display")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Display")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
	int32 ModuleId = 0;
};
