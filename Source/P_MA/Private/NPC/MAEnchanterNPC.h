#pragma once

#include "CoreMinimal.h"
#include "NPC/MANPC.h"
#include "MAEnchanterNPC.generated.h"

class APlayerController;
class AMAPlayerCharacter;
class UMAEnchanterWidget;
class UMASkillModuleInstance;

UCLASS()
class P_MA_API AMAEnchanterNPC : public AMANPC
{
	GENERATED_BODY()

public:
	AMAEnchanterNPC();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool EnchantModule(
		APlayerController* PlayerController,
		UMASkillModuleInstance* TargetModule,
		int32 RuneEntryId,
		int32 EnchantmentSlotIndex);
	int32 GetEnchantSlotCount() const;
	void CloseEnchanter();

private:
	void HandleInteract(AMAPlayerCharacter* Interactor);

	UPROPERTY(EditDefaultsOnly, Category="Enchantment")
	TSubclassOf<UMAEnchanterWidget> EnchanterWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UMAEnchanterWidget> ActiveEnchanterWidget = nullptr;

	UPROPERTY(Transient)
	TWeakObjectPtr<AMAPlayerCharacter> ActiveInteractor;
};
