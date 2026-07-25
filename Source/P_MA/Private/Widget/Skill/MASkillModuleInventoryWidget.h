#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MASkillModuleInventoryWidget.generated.h"

class UPanelWidget;
class UWidgetAnimation;
class UMAInventoryComponent;
class UMASkillModuleSocketWidget;

UCLASS()
class P_MA_API UMASkillModuleInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeInventory(UMAInventoryComponent* InInventory);
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
	void RefreshSlots();
	void UnbindInventory();
	void EnsureSlotWidgets(int32 TargetCount);

	UPROPERTY(Transient)
	TObjectPtr<UMAInventoryComponent> Inventory = nullptr;

	FDelegateHandle InventoryChangedHandle;
	bool bIsCollapsed = false;
};
