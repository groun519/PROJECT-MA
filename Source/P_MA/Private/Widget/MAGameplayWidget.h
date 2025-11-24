#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MAGameplayWidget.generated.h"

class UMASkillSlotWidget;
class UMAPassiveSlotWidget;
class UHorizontalBox;
class UMAValueGauge;
class UMAMobilityChargeWidget;
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
	
	void SetOwinigPawnInputEnabled(bool bPawnInputEnabled);
protected:
	UPROPERTY(meta = (BindWidget))
	class UMAValueGauge* HealthBar;

	UPROPERTY(meta=(BindWidget))
	class UMAAbilityListView* AbilityListView;

	UPROPERTY(Transient, meta=(BindWidgetAnim))
	class UWidgetAnimation* ShopPopupAnimation;

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
private:

	UFUNCTION()
	void OnShopButtonClicked();
};
