#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Item/MAItemTypes.h"
#include "MAItemData.generated.h"

class UGameplayEffect;
class UTexture2D;

USTRUCT(BlueprintType)
struct P_MA_API FMAItemDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Display")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Display", meta=(MultiLine="true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Display")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop", meta=(ClampMin="0"))
	int32 Price = 0;
};

USTRUCT(BlueprintType)
struct P_MA_API FMAConsumableItemDataRow : public FMAItemDataRow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable")
	TSoftClassPtr<UGameplayEffect> UseEffect;
};

USTRUCT(BlueprintType)
struct P_MA_API FMARuneItemDataRow : public FMAItemDataRow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rune")
	int32 ModuleId = 0;
};
