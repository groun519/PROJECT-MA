#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MAGameplayWidget.generated.h"

class UMAValueGauge;
class ULoopReadyWidget;
class UShopWidget; 
class USkillBookWidget; 
class UMASkillSlotWidget;

UCLASS()
class UMAGameplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void ConfigureAbilities(const TMap<EMAAbilityInputID, TSubclassOf<UGameplayAbility>>& Abilities);

	USkillBookWidget* GetSkillBookWidget() const { return ActiveSkillBookWidget; }
	
	void ToggleShop();
	void ToggleSkillBook();
	void ToggleSkillSlotsCollapsed();

	// Loop Ready UI
	void SetLoopReadyVisible(bool bVisible);
	void RefreshLoopReady();

protected:
	UPROPERTY(meta = (BindWidget))
	class UMAValueGauge* HealthBar;

	UPROPERTY(meta=(BindWidget))
	class UMAAbilityListView* AbilityListView;

	UPROPERTY(EditDefaultsOnly, Category = "Shop UI")
	TSubclassOf<class UShopWidget> ShopWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Shop UI")
	TArray<class UDataTable*> ShopDataTables;

	UPROPERTY()
	class UShopWidget* ActiveShopWidget;

	UPROPERTY(meta=(BindWidget))
	class UButton *ShopButton;
	
	UPROPERTY(EditDefaultsOnly, Category = "Skill UI")
	TSubclassOf<class USkillBookWidget> SkillBookWidgetClass;

	UPROPERTY()
	class USkillBookWidget* ActiveSkillBookWidget;

	UPROPERTY(meta=(BindWidget))
	class UInventoryWidget* InventoryWidget;

	UPROPERTY(meta = (BindWidget))
	ULoopReadyWidget* LoopReadyWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMASkillSlotWidget> SkillSlotWidget;
	
	bool bLoopReadyInitialized = false;

	UFUNCTION()
	void OnShopButtonClicked();
};
