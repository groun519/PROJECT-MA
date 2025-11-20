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
	// 체력바와 같은 기존 UI 요소 유지
	UPROPERTY(meta = (BindWidget))
	class UMAValueGauge* HealthBar;

	// // 스킬 슬롯 위젯 클래스와 바인딩
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	// TSubclassOf<UMASkillSlotWidget> SkillSlotWidgetClass;
	//
	// UPROPERTY(meta = (BindWidget))
	// UHorizontalBox* HorizontalBox_SkillSlots;
	//
	// // 패시브 슬롯 위젯 클래스와 바인딩
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	// TSubclassOf<UMAPassiveSlotWidget> PassiveSlotWidgetClass;
	//
	// UPROPERTY(meta = (BindWidget))
	// UHorizontalBox* HorizontalBox_PassiveSlots;

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
	// void CreateSkillSlots(int32 NumSlots);
	// void CreatePassiveSlots(int32 NumSlots);
	
	UFUNCTION()
	void OnShopButtonClicked();
};
