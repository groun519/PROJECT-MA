#pragma once

#include "CoreMinimal.h"
#include "MAItemTypes.generated.h"

class UMAItemType;

UENUM(BlueprintType)
enum class EMAItemUseResult : uint8
{
	Success,
	NotUsable,
	InvalidEntry,
	InvalidData,
	Failed
};

USTRUCT(BlueprintType)
struct P_MA_API FMAItemId
{
	GENERATED_BODY()

	FMAItemId() = default;
	FMAItemId(TSubclassOf<UMAItemType> InType, const FName InRowName)
		: Type(InType), RowName(InRowName) {}

	bool IsValid() const;
	const UMAItemType* GetItemType() const;
	bool operator==(const FMAItemId& Other) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	TSubclassOf<UMAItemType> Type;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FName RowName = NAME_None;
};
