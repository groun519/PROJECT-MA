#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MAOverHeadStatsGauge.generated.h"

class UMAValueGauge;

UCLASS()
class UMAOverHeadStatsGauge : public UUserWidget
{
	GENERATED_BODY()
public:
	void InitializeFromCharacter(class AMACharacter* OwnerCharacter);
	void RefreshStatusEffectDisplay();

private:
	void ConfigureWithASC(class UAbilitySystemComponent* AbilitySystemComponent);
	void ConfigureWithStatusEffectComponent(class UMAStatusEffectComponent* StatusEffectComponent);
	void UnbindFromStatusEffectComponent();
	TArray<class UMAStatusEffectDurationWidget*> GetStatusEffectSlots() const;
	void ClearStatusEffectSlots() const;

	UPROPERTY(meta = (BindWidget))
	UMAValueGauge* HealthBar;
	
	UPROPERTY(meta = (BindWidgetOptional))
	UMAValueGauge* FuryBar;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UMAStatusEffectDurationWidget> StatusEffectSlot0;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UMAStatusEffectDurationWidget> StatusEffectSlot1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UMAStatusEffectDurationWidget> StatusEffectSlot2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UMAStatusEffectDurationWidget> StatusEffectSlot3;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UMAStatusEffectDurationWidget> StatusEffectSlot4;
	TWeakObjectPtr<class UMAStatusEffectComponent> BoundStatusEffectComponent;
};
