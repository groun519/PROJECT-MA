#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MASkillModuleInventoryWidget.generated.h"

class UPanelWidget;
class UWidgetAnimation;
class UMASkillModuleInventoryComponent;
class UMASkillModuleSocketWidget;

UCLASS()
class P_MA_API UMASkillModuleInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeInventory(UMASkillModuleInventoryComponent* InInventory);
	void RefreshSlots();
	void ToggleCollapsed();
	void SetCollapsed(bool bCollapsed);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> SlotContainer;

	UPROPERTY(Transient, meta=(BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> InventoryCollapse;

	UPROPERTY(EditDefaultsOnly, Category="Skill")
	TSubclassOf<UMASkillModuleSocketWidget> SlotWidgetClass;

private:
	void UnbindInventory();
	void EnsureSlotWidgets(int32 TargetCount);

	UPROPERTY(Transient)
	TObjectPtr<UMASkillModuleInventoryComponent> Inventory = nullptr;

	FDelegateHandle InventoryChangedHandle;
	bool bIsCollapsed = false;
};
