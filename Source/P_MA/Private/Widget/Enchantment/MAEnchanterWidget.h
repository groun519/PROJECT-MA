#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "MAEnchanterWidget.generated.h"

class AMAEnchanterNPC;
class UMAEnchantmentCompositionWidget;
class UMAEnchantmentEntryWidget;
class UMAInventoryComponent;
class UMASkillManagerComponent;
class UMASkillModuleInstance;
class UButton;
class UPanelWidget;

UCLASS()
class P_MA_API UMAEnchanterWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnPreviewKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent) override;

	void InitializeEnchanter(AMAEnchanterNPC* InEnchanterNPC);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> TargetContainer;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMAEnchantmentCompositionWidget> CompositionWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> RuneContainer;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> EnchantButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(EditDefaultsOnly, Category="Enchantment")
	TSubclassOf<UMAEnchantmentEntryWidget> EntryWidgetClass;

private:
	void BindPlayerState();
	void UnbindPlayerState();
	void RefreshTargets();
	void RefreshComposition();
	void RefreshRunes();
	void RefreshControls();
	void SetSelectedTarget(UMASkillModuleInstance* TargetModule);
	void SelectTarget(UMASkillModuleInstance* TargetModule);
	void SelectEnchantmentSlot(int32 SlotIndex);
	void SelectRune(int32 RuneEntryId);

	void HandleInventoryChanged();
	void HandleSkillSlotChanged(FGameplayTag SlotTag);
	void HandleSubModulesChanged(UMASkillModuleInstance* ModuleInstance);
	void HandleEnchantCompleted(bool bSucceeded);

	UFUNCTION()
	void HandleEnchantButtonClicked();

	UFUNCTION()
	void HandleCloseButtonClicked();

	UPROPERTY(Transient)
	TObjectPtr<AMAEnchanterNPC> EnchanterNPC = nullptr;

	TWeakObjectPtr<UMAInventoryComponent> Inventory;
	TWeakObjectPtr<UMASkillManagerComponent> SkillManager;
	TWeakObjectPtr<UMASkillModuleInstance> SelectedTarget;
	TMap<TWeakObjectPtr<UMASkillModuleInstance>, TWeakObjectPtr<UMAEnchantmentEntryWidget>> TargetEntries;
	TMap<int32, TWeakObjectPtr<UMAEnchantmentEntryWidget>> RuneEntries;

	FDelegateHandle InventoryChangedHandle;
	FDelegateHandle SkillSlotChangedHandle;
	FDelegateHandle SubModulesChangedHandle;
	FDelegateHandle EnchantCompletedHandle;
	int32 SelectedEnchantmentSlotIndex = INDEX_NONE;
	int32 SelectedRuneEntryId = INDEX_NONE;
	bool bRequestPending = false;
};
