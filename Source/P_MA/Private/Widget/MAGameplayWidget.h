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
UCLASS()
class UMAGameplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void ConfigureAbilities(const TMap<EMAAbilityInputID, TSubclassOf<class UGameplayAbility>>& Abilities);

	class USkillBookWidget* GetSkillBookWidget() const { return SkillBookWidget; }
	
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

	UPROPERTY(Transient, meta=(BindWidgetAnim))
	class UWidgetAnimation* ShopPopupAnimation;

	UPROPERTY(Transient, meta=(BindWidgetAnim))
	class UWidgetAnimation* SkillBookPopupAnimation;

	void PlayShopPopupAnimation(bool bPlayForward);

	UPROPERTY(meta = (BindWidget))
	UMAMobilityChargeWidget* ChargeBar;

	UPROPERTY(meta=(BindWidget))
	class UShopWidget* ShopWidget;

	UPROPERTY(meta=(BindWidget))
	class UInventoryWidget* InventoryWidget;

	UPROPERTY(meta=(BindWidget))
	class UButton *ShopButton;
	
	UPROPERTY(meta=(BindWidget))
	class USkillBookWidget* SkillBookWidget;

	UPROPERTY(meta = (BindWidget))
	ULoopReadyWidget* LoopReadyWidget;
	
	bool bLoopReadyInitialized = false;
private:

	UFUNCTION()
	void OnShopButtonClicked();
};
