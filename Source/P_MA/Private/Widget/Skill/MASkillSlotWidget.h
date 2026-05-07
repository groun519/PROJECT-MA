#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MASkillSlotWidget.generated.h"

class UPanelWidget;
class UMASkillManagerComponent;
class UMASkillSlotRowWidget;

UCLASS()
class P_MA_API UMASkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeSkillSlots(UMASkillManagerComponent* InSkillManager);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> SlotRowsBox;

	UPROPERTY(EditDefaultsOnly, Category="Skill")
	TSubclassOf<UMASkillSlotRowWidget> SlotRowWidgetClass;

private:
	void RebuildSlotRows();

	UPROPERTY(Transient)
	TObjectPtr<UMASkillManagerComponent> SkillManager = nullptr;
};
