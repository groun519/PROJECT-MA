#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MASkillSlotRowWidget.generated.h"

class UMASkillIconWidget;
class UMASkillDefinition;
class UMASkillManagerComponent;
class UMASkillModuleInstance;
class UMASkillModuleSocketWidget;
class AMAPlayerController;
class UHorizontalBox;
class UWidgetAnimation;

UCLASS()
class P_MA_API UMASkillSlotRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeSlot(UMASkillManagerComponent* InSkillManager, EMAAbilityInputID InInputID);

	void Refresh();
	void SetCollapsed(bool bCollapsed);

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMASkillIconWidget> SkillIconWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> ModuleSocketBox;

	UPROPERTY(EditDefaultsOnly, Category="Skill")
	TSubclassOf<UMASkillModuleSocketWidget> ModuleSocketWidgetClass;

	UPROPERTY(Transient, meta=(BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> RowCollapse;

private:
	void HandleSkillSlotChanged(EMAAbilityInputID ChangedInputID);
	void RefreshHotkeyText();

	void RebuildModuleSockets(const TArray<TObjectPtr<UMASkillModuleInstance>>* InModuleInstances);

	UPROPERTY(Transient)
	TObjectPtr<UMASkillManagerComponent> SkillManager = nullptr;

	UPROPERTY(Transient)
	EMAAbilityInputID InputID = EMAAbilityInputID::None;

	bool bIsCollapsed = false;

	TWeakObjectPtr<AMAPlayerController> InputBindingsOwner;
};
