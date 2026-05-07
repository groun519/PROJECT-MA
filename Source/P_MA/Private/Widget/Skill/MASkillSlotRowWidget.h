#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MASkillSlotRowWidget.generated.h"

class UMASkillIconWidget;
class UMASkillDefinition;
class UMASkillManagerComponent;
class UMASkillModuleSocketWidget;
class UHorizontalBox;

UCLASS()
class P_MA_API UMASkillSlotRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeSlot(UMASkillManagerComponent* InSkillManager, EMAAbilityInputID InInputID);

	void Refresh();

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMASkillIconWidget> SkillIconWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> ModuleSocketBox;

	UPROPERTY(EditDefaultsOnly, Category="Skill")
	TSubclassOf<UMASkillModuleSocketWidget> ModuleSocketWidgetClass;

private:
	void HandleSkillSlotChanged(EMAAbilityInputID ChangedInputID);

	void RebuildModuleSockets(const TArray<UMASkillDefinition*>& InSkillDefinitions);

	UPROPERTY(Transient)
	TObjectPtr<UMASkillManagerComponent> SkillManager = nullptr;

	UPROPERTY(Transient)
	EMAAbilityInputID InputID = EMAAbilityInputID::None;
};
