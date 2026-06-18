#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MAGameplayWidget.generated.h"

class UMAValueGauge;
class ULoopReadyWidget;
class UButton;
class UMASkillModuleInventoryWidget;
class UMASkillSlotWidget;
class UMATemperatureGauge;

UCLASS()
class UMAGameplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void ToggleModuleInventory();

	void ToggleSkillSlotsCollapsed();

	// Loop Ready UI
	void SetLoopReadyVisible(bool bVisible);
	void RefreshLoopReady();

protected:
	UPROPERTY(meta = (BindWidget))
	class UMAValueGauge* HealthBar;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UMATemperatureGauge> TemperatureBar;

	UPROPERTY(meta=(BindWidget))
	class UInventoryWidget* InventoryWidget;

	UPROPERTY(meta = (BindWidget))
	ULoopReadyWidget* LoopReadyWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMASkillSlotWidget> SkillSlotWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMASkillModuleInventoryWidget> SkillModuleInventoryWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ModuleInventoryToggleButton;
	
	bool bLoopReadyInitialized = false;
};
