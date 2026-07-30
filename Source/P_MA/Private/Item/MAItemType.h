#pragma once

#include "CoreMinimal.h"
#include "Item/MAItemTypes.h"
#include "MAItemType.generated.h"

class AActor;
struct FMAItemDataRow;
class UDataTable;

/** Shared definition for one item data family and its direct-use behavior. */
UCLASS(Abstract, BlueprintType)
class P_MA_API UMAItemType : public UObject
{
	GENERATED_BODY()

public:
	const FMAItemDataRow* FindItemData(FName RowName) const;
	virtual EMAItemUseResult TryUse(AActor& OwnerActor, FName RowName) const
	{
		return EMAItemUseResult::NotUsable;
	}

protected:
	virtual const UDataTable* LoadItemDataTable() const PURE_VIRTUAL(
		UMAItemType::LoadItemDataTable,
		return nullptr;);
};

UCLASS(BlueprintType, Config=Game, DefaultConfig)
class P_MA_API UMARuneItemType : public UMAItemType
{
	GENERATED_BODY()

protected:
	virtual const UDataTable* LoadItemDataTable() const override;

private:
	UPROPERTY(Config, EditDefaultsOnly, Category="Item", meta=(RowType="/Script/P_MA.MARuneItemDataRow"))
	TSoftObjectPtr<UDataTable> ItemDataTable;
};

UCLASS(BlueprintType, Config=Game, DefaultConfig)
class P_MA_API UMAConsumableItemType : public UMAItemType
{
	GENERATED_BODY()

public:
	virtual EMAItemUseResult TryUse(AActor& OwnerActor, FName RowName) const override;

protected:
	virtual const UDataTable* LoadItemDataTable() const override;

private:
	UPROPERTY(Config, EditDefaultsOnly, Category="Item", meta=(RowType="/Script/P_MA.MAConsumableItemDataRow"))
	TSoftObjectPtr<UDataTable> ItemDataTable;
};
