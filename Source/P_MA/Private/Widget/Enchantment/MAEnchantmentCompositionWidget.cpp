#include "Widget/Enchantment/MAEnchantmentCompositionWidget.h"

#include "Components/PanelWidget.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "Rendering/DrawElements.h"
#include "Widget/Enchantment/MAEnchantmentNodeWidget.h"

int32 UMAEnchantmentCompositionWidget::NativePaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	const int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	const bool bParentEnabled) const
{
	if (RootModuleEntry)
	{
		const FVector2D Start = GetRightAnchor(*RootModuleEntry, AllottedGeometry);
		for (int32 SlotIndex = 0; SlotIndex < SlotEntries.Num(); ++SlotIndex)
		{
			const UMAEnchantmentNodeWidget* SlotEntry = SlotEntries[SlotIndex];
			if (!SlotEntry) continue;

			const FVector2D End = GetLeftAnchor(*SlotEntry, AllottedGeometry);
			const float TangentLength = FMath::Max(64.f, (End.X - Start.X) * 0.55f);
			const bool bSelected = SlotIndex == SelectedSlotIndex;
			FSlateDrawElement::MakeSpline(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(),
				Start,
				FVector2D(TangentLength, 0.f),
				End,
				FVector2D(TangentLength, 0.f),
				bSelected ? SelectedConnectionThickness : ConnectionThickness,
				ESlateDrawEffect::None,
				bSelected ? SelectedConnectionColor : ConnectionColor);
		}
	}

	return Super::NativePaint(
		Args,
		AllottedGeometry,
		MyCullingRect,
		OutDrawElements,
		LayerId + 2,
		InWidgetStyle,
		bParentEnabled);
}

void UMAEnchantmentCompositionWidget::SetComposition(
	UMASkillModuleInstance* ModuleInstance,
	const int32 SlotCount,
	const int32 InSelectedSlotIndex)
{
	RootModuleContainer->ClearChildren();
	SlotContainer->ClearChildren();
	RootModuleEntry = nullptr;
	SlotEntries.Reset();
	SelectedSlotIndex = InSelectedSlotIndex;
	if (!NodeWidgetClass) return;

	RootModuleEntry = CreateWidget<UMAEnchantmentNodeWidget>(this, NodeWidgetClass);
	if (RootModuleEntry)
	{
		if (ModuleInstance && ModuleInstance->GetRootModule())
		{
			RootModuleEntry->SetModuleInstance(*ModuleInstance);
		}
		else
		{
			RootModuleEntry->SetEmpty();
		}
		RootModuleEntry->SetSelected(false);
		RootModuleContainer->AddChild(RootModuleEntry);
	}

	const TArray<TObjectPtr<UMASkillModule>>* SubModules = ModuleInstance
		? &ModuleInstance->GetModuleGroup().SubModules
		: nullptr;
	for (int32 SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
	{
		UMAEnchantmentNodeWidget* SlotEntry = CreateWidget<UMAEnchantmentNodeWidget>(this, NodeWidgetClass);
		if (!SlotEntry) continue;

		if (SubModules && SubModules->IsValidIndex(SlotIndex) && (*SubModules)[SlotIndex])
		{
			SlotEntry->SetModule(*(*SubModules)[SlotIndex]);
		}
		else
		{
			SlotEntry->SetEmpty();
		}
		SlotEntry->SetSelected(SelectedSlotIndex == SlotIndex);
		if (ModuleInstance)
		{
			SlotEntry->OnSelected.AddWeakLambda(this, [this, SlotIndex]()
			{
				OnSlotSelected.Broadcast(SlotIndex);
			});
		}
		SlotContainer->AddChild(SlotEntry);
		SlotEntries.Add(SlotEntry);
	}
}

void UMAEnchantmentCompositionWidget::SetSelectedSlot(const int32 SlotIndex)
{
	SelectedSlotIndex = SlotIndex;
	for (int32 Index = 0; Index < SlotEntries.Num(); ++Index)
	{
		if (SlotEntries[Index]) SlotEntries[Index]->SetSelected(Index == SlotIndex);
	}
	Invalidate(EInvalidateWidgetReason::Paint);
}

FVector2D UMAEnchantmentCompositionWidget::GetRightAnchor(
	const UWidget& Widget,
	const FGeometry& RootGeometry) const
{
	const FGeometry& Geometry = Widget.GetPaintSpaceGeometry();
	const FVector2D Size = Geometry.GetLocalSize();
	return RootGeometry.AbsoluteToLocal(Geometry.LocalToAbsolute(FVector2D(Size.X, Size.Y * 0.5f)));
}

FVector2D UMAEnchantmentCompositionWidget::GetLeftAnchor(
	const UWidget& Widget,
	const FGeometry& RootGeometry) const
{
	const FGeometry& Geometry = Widget.GetPaintSpaceGeometry();
	const FVector2D Size = Geometry.GetLocalSize();
	return RootGeometry.AbsoluteToLocal(Geometry.LocalToAbsolute(FVector2D(0.f, Size.Y * 0.5f)));
}
