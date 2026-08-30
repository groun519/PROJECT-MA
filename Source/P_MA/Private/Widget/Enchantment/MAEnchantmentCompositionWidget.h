#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MAEnchantmentCompositionWidget.generated.h"

class UMAEnchantmentNodeWidget;
class UMASkillModuleInstance;
class UPanelWidget;

DECLARE_MULTICAST_DELEGATE_OneParam(FMAEnchantmentSlotSelectedSignature, int32);

UCLASS()
class P_MA_API UMAEnchantmentCompositionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual int32 NativePaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

	void SetComposition(
		UMASkillModuleInstance* ModuleInstance,
		int32 SlotCount,
		int32 SelectedSlotIndex);
	void SetSelectedSlot(int32 SlotIndex);

	FMAEnchantmentSlotSelectedSignature OnSlotSelected;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Enchantment")
	TSubclassOf<UMAEnchantmentNodeWidget> NodeWidgetClass;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> RootModuleContainer;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> SlotContainer;

	UPROPERTY(EditDefaultsOnly, Category="Appearance")
	FLinearColor ConnectionColor = FLinearColor(0.38f, 0.42f, 0.44f, 0.7f);

	UPROPERTY(EditDefaultsOnly, Category="Appearance")
	FLinearColor SelectedConnectionColor = FLinearColor(0.78f, 0.82f, 0.84f, 0.9f);

	UPROPERTY(EditDefaultsOnly, Category="Appearance", meta=(ClampMin="0.0"))
	float ConnectionThickness = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category="Appearance", meta=(ClampMin="0.0"))
	float SelectedConnectionThickness = 2.5f;

private:
	FVector2D GetRightAnchor(const UWidget& Widget, const FGeometry& RootGeometry) const;
	FVector2D GetLeftAnchor(const UWidget& Widget, const FGeometry& RootGeometry) const;

	UPROPERTY(Transient)
	TObjectPtr<UMAEnchantmentNodeWidget> RootModuleEntry = nullptr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMAEnchantmentNodeWidget>> SlotEntries;

	int32 SelectedSlotIndex = INDEX_NONE;
};
