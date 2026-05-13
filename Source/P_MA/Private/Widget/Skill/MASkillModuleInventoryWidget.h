#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MASkillModuleInventoryWidget.generated.h"

class UPanelWidget;
class UMASkillModuleInventoryComponent;
class UMASkillModuleSocketWidget;

UCLASS()
class P_MA_API UMASkillModuleInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeInventory(UMASkillModuleInventoryComponent* InInventory);
	void RefreshSlots();

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> SlotContainer;

	UPROPERTY(EditDefaultsOnly, Category="Skill")
	TSubclassOf<UMASkillModuleSocketWidget> SlotWidgetClass;

private:
	void UnbindInventory();
	void EnsureSlotWidgets(int32 TargetCount);

	UPROPERTY(Transient)
	TObjectPtr<UMASkillModuleInventoryComponent> Inventory = nullptr;

	FDelegateHandle InventoryChangedHandle;
};
