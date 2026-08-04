#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MAAreaDecalData.generated.h"

class UMaterialInterface;

USTRUCT(BlueprintType)
struct P_MA_API FMAAreaDecalDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Decal")
	TObjectPtr<UMaterialInterface> DecalMaterial = nullptr;
};
