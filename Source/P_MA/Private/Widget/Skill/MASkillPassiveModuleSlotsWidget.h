#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "MASkillPassiveModuleSlotsWidget.generated.h"

class UPanelWidget;
class UMASkillManagerComponent;
class UMASkillModuleSocketWidget;

UCLASS()
class P_MA_API UMASkillPassiveModuleSlotsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializePassiveSlots(UMASkillManagerComponent* InSkillManager);

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> SlotContainer;

	UPROPERTY(EditDefaultsOnly, Category="Skill")
	TSubclassOf<UMASkillModuleSocketWidget> SlotWidgetClass;

private:
	void RefreshSlots();
	void HandleSkillSlotChanged(FGameplayTag ChangedSlotTag);
	void UnbindSkillManager();
	void EnsureSlotWidgets(int32 TargetCount);

	UPROPERTY(Transient)
	TObjectPtr<UMASkillManagerComponent> SkillManager = nullptr;

	FDelegateHandle SkillSlotChangedHandle;
};
