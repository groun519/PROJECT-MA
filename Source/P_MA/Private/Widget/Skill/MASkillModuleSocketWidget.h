#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MASkillModuleSocketWidget.generated.h"

class UImage;
class UMASkillDefinition;
class UMASkillManagerComponent;
class UMASkillModuleDragVisualWidget;

UCLASS()
class P_MA_API UMASkillModuleSocketWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeSocket(UMASkillManagerComponent* InSkillManager, EMAAbilityInputID InInputID, int32 InModuleIndex, UMASkillDefinition* InSkillDefinition);

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ModuleIconImage;

	UPROPERTY(EditDefaultsOnly, Category="Skill")
	TSubclassOf<UMASkillModuleDragVisualWidget> DragVisualWidgetClass;

private:
	UPROPERTY(Transient)
	TObjectPtr<UMASkillManagerComponent> SkillManager = nullptr;

	UPROPERTY(Transient)
	EMAAbilityInputID InputID = EMAAbilityInputID::None;

	UPROPERTY(Transient)
	int32 ModuleIndex = INDEX_NONE;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillDefinition> SkillDefinition = nullptr;
};
