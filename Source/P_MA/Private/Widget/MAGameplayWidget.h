#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "Widget/Loop/LoopPlayerStatusWidget.h"
#include "MAGameplayWidget.generated.h"

class UMASkillSlotWidget;
class UMAPassiveSlotWidget;
class UHorizontalBox;
class UMAValueGauge;
class UMAMobilityChargeWidget;
class ULoopReadyWidget;
class UShopWidget; 
class USkillBookWidget; 

UCLASS()
class UMAGameplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	void ConfigureAbilities(const TMap<EMAAbilityInputID, TSubclassOf<class UGameplayAbility>>& Abilities);

	// 💡 Getter 함수가 이제 'ActiveSkillBookWidget'을 반환하도록 수정
	class USkillBookWidget* GetSkillBookWidget() const { return ActiveSkillBookWidget; }
	
	void ToggleShop();
	void ToggleSkillBook();

	// Loop Ready UI
	void SetLoopReadyVisible(bool bVisible);
	void RefreshLoopReady();

protected:
	UPROPERTY(meta = (BindWidget))
	class UMAValueGauge* HealthBar;

	UPROPERTY(meta=(BindWidget))
	class UMAAbilityListView* AbilityListView;
	
	UPROPERTY(meta = (BindWidget))
	UMAMobilityChargeWidget* ChargeBar;
	
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
	
	bool bLoopReadyInitialized = false;

	UFUNCTION()
	void OnShopButtonClicked();
};
