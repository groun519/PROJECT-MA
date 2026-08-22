#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "MACheatManager.generated.h"

class AMAPlayerCharacter;

UCLASS()
class P_MA_API UMACheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	UFUNCTION(Exec)
	void AddCoin(float Amount = 1000.f);

	UFUNCTION(Exec)
	void RefreshShopStock();

	UFUNCTION(Exec)
	void ShopTest();

	UFUNCTION(Exec)
	void SetMAState(int32 NewState);

	UFUNCTION(Exec)
	void AddModule(int32 ModuleId, int32 Count = 1);

	UFUNCTION(Exec)
	void ListItems();

	UFUNCTION(Exec)
	void UseItem(int32 EntryId);

	UFUNCTION(Exec)
	void EnchantModule(FString SlotTagName, int32 ModuleIndex, int32 RuneEntryId);

private:
	AMAPlayerCharacter* GetMAPlayerCharacter() const;
};
