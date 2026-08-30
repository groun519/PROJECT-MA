#include "Widget/Enchantment/MAEnchanterWidget.h"

#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/MASkillSystemTypes.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "InputCoreTypes.h"
#include "Inventory/MAInventoryComponent.h"
#include "Inventory/MAInventoryTypes.h"
#include "NPC/MAEnchanterNPC.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/MAPlayerController.h"
#include "Widget/Enchantment/MAEnchantmentCompositionWidget.h"
#include "Widget/Enchantment/MAEnchantmentEntryWidget.h"
#include "Widget/Enchantment/MAEnchantmentNodeWidget.h"

void UMAEnchanterWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);

	EnchantButton->OnClicked.RemoveDynamic(this, &UMAEnchanterWidget::HandleEnchantButtonClicked);
	EnchantButton->OnClicked.AddDynamic(this, &UMAEnchanterWidget::HandleEnchantButtonClicked);
	CloseButton->OnClicked.RemoveDynamic(this, &UMAEnchanterWidget::HandleCloseButtonClicked);
	CloseButton->OnClicked.AddDynamic(this, &UMAEnchanterWidget::HandleCloseButtonClicked);

	CompositionWidget->OnSlotSelected.RemoveAll(this);
	CompositionWidget->OnSlotSelected.AddUObject(
		this,
		&UMAEnchanterWidget::SelectEnchantmentSlot);

	EnchantButton->SetIsEnabled(false);
}

void UMAEnchanterWidget::NativeDestruct()
{
	CompositionWidget->OnSlotSelected.RemoveAll(this);
	UnbindPlayerState();
	Super::NativeDestruct();
}

FReply UMAEnchanterWidget::NativeOnPreviewKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		HandleCloseButtonClicked();
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void UMAEnchanterWidget::InitializeEnchanter(AMAEnchanterNPC* InEnchanterNPC)
{
	EnchanterNPC = InEnchanterNPC;
	BindPlayerState();
	RefreshTargets();
	RefreshComposition();
	RefreshRunes();
	RefreshControls();
}

void UMAEnchanterWidget::BindPlayerState()
{
	UnbindPlayerState();

	AMAPlayerCharacter* PlayerCharacter = GetOwningPlayerPawn<AMAPlayerCharacter>();
	Inventory = PlayerCharacter ? PlayerCharacter->GetInventoryComponent() : nullptr;
	SkillManager = PlayerCharacter ? PlayerCharacter->GetSkillManagerComponent() : nullptr;

	if (Inventory.IsValid())
	{
		InventoryChangedHandle = Inventory->OnInventoryChanged.AddUObject(
			this,
			&UMAEnchanterWidget::HandleInventoryChanged);
	}
	if (SkillManager.IsValid())
	{
		SkillSlotChangedHandle = SkillManager->OnSkillSlotChanged.AddUObject(
			this,
			&UMAEnchanterWidget::HandleSkillSlotChanged);
	}
	if (AMAPlayerController* PlayerController = GetOwningPlayer<AMAPlayerController>())
	{
		EnchantCompletedHandle = PlayerController->OnEnchantCompleted.AddUObject(
			this,
			&UMAEnchanterWidget::HandleEnchantCompleted);
	}
}

void UMAEnchanterWidget::UnbindPlayerState()
{
	if (SelectedTarget.IsValid() && SubModulesChangedHandle.IsValid())
	{
		SelectedTarget->OnSubModulesChanged.Remove(SubModulesChangedHandle);
	}
	if (Inventory.IsValid() && InventoryChangedHandle.IsValid())
	{
		Inventory->OnInventoryChanged.Remove(InventoryChangedHandle);
	}
	if (SkillManager.IsValid() && SkillSlotChangedHandle.IsValid())
	{
		SkillManager->OnSkillSlotChanged.Remove(SkillSlotChangedHandle);
	}
	if (AMAPlayerController* PlayerController = GetOwningPlayer<AMAPlayerController>();
		PlayerController && EnchantCompletedHandle.IsValid())
	{
		PlayerController->OnEnchantCompleted.Remove(EnchantCompletedHandle);
	}

	InventoryChangedHandle.Reset();
	SkillSlotChangedHandle.Reset();
	SubModulesChangedHandle.Reset();
	EnchantCompletedHandle.Reset();
	Inventory.Reset();
	SkillManager.Reset();
}

void UMAEnchanterWidget::RefreshTargets()
{
	TargetContainer->ClearChildren();
	TargetEntries.Reset();
	if (!SkillManager.IsValid() || !EntryWidgetClass)
	{
		SetSelectedTarget(nullptr);
		return;
	}

	TArray<FGameplayTag> SlotTags = SkillManager->GetSkillSlotTags();
	SlotTags.AddUnique(FMASkillSystemStatics::GetPassiveSlotTag());
	bool bSelectedTargetExists = false;
	for (const FGameplayTag& SlotTag : SlotTags)
	{
		const FText SlotText = FMASkillSystemStatics::IsPassiveSkillSlotTag(SlotTag)
			? NSLOCTEXT("MAEnchanterWidget", "PassiveSlot", "Passive")
			: FText::Format(
				NSLOCTEXT("MAEnchanterWidget", "ActiveSlot", "Skill {0}"),
				FText::AsNumber(FMASkillSystemStatics::ResolveSlotInputID(SlotTag)));

		const int32 SlotCount = SkillManager->GetModuleSlotCount(SlotTag);
		for (int32 SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
		{
			UMASkillModuleInstance* ModuleInstance = SkillManager->GetModuleInstanceAt(SlotTag, SlotIndex);
			if (!ModuleInstance || !ModuleInstance->GetRootModule()) continue;

			UMAEnchantmentEntryWidget* EntryWidget = CreateWidget<UMAEnchantmentEntryWidget>(this, EntryWidgetClass);
			if (!EntryWidget) continue;

			EntryWidget->SetModuleInstance(
				*ModuleInstance,
				FText::Format(
					NSLOCTEXT("MAEnchanterWidget", "ModulePosition", "{0}-{1}"),
					SlotText,
					FText::AsNumber(SlotIndex + 1)));
			EntryWidget->SetSelected(SelectedTarget.Get() == ModuleInstance);
			TWeakObjectPtr<UMASkillModuleInstance> WeakModuleInstance = ModuleInstance;
			EntryWidget->OnSelected.AddWeakLambda(this, [this, WeakModuleInstance]()
			{
				SelectTarget(WeakModuleInstance.Get());
			});
			TargetContainer->AddChild(EntryWidget);
			TargetEntries.Add(ModuleInstance, EntryWidget);
			bSelectedTargetExists |= SelectedTarget.Get() == ModuleInstance;
		}
	}

	if (!bSelectedTargetExists) SetSelectedTarget(nullptr);
}

void UMAEnchanterWidget::RefreshComposition()
{
	if (!EnchanterNPC)
	{
		SelectedEnchantmentSlotIndex = INDEX_NONE;
		CompositionWidget->SetComposition(nullptr, 0, INDEX_NONE);
		return;
	}

	const int32 SlotCount = EnchanterNPC->GetEnchantSlotCount();
	if (!SelectedTarget.IsValid() || !SelectedTarget->GetRootModule())
	{
		SelectedEnchantmentSlotIndex = INDEX_NONE;
		CompositionWidget->SetComposition(nullptr, SlotCount, INDEX_NONE);
		return;
	}

	const TArray<TObjectPtr<UMASkillModule>>& SubModules = SelectedTarget->GetModuleGroup().SubModules;
	if (SelectedEnchantmentSlotIndex < 0 || SelectedEnchantmentSlotIndex >= SlotCount)
	{
		SelectedEnchantmentSlotIndex = INDEX_NONE;
		for (int32 SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
		{
			if (!SubModules.IsValidIndex(SlotIndex) || !SubModules[SlotIndex])
			{
				SelectedEnchantmentSlotIndex = SlotIndex;
				break;
			}
		}
		if (SelectedEnchantmentSlotIndex == INDEX_NONE && SlotCount > 0)
		{
			SelectedEnchantmentSlotIndex = 0;
		}
	}
	CompositionWidget->SetComposition(
		SelectedTarget.Get(),
		SlotCount,
		SelectedEnchantmentSlotIndex);
}

void UMAEnchanterWidget::RefreshRunes()
{
	RuneContainer->ClearChildren();
	RuneEntries.Reset();
	if (!Inventory.IsValid() || !EntryWidgetClass)
	{
		SelectedRuneEntryId = INDEX_NONE;
		return;
	}

	bool bSelectedRuneExists = false;
	for (const FMAInventoryEntry& Entry : Inventory->GetEntries())
	{
		const FMAInventoryStack* Stack = Entry.GetStack();
		if (!Stack || Stack->Module->GetModuleType() != EMASkillModuleType::Sub) continue;

		UMAEnchantmentEntryWidget* EntryWidget = CreateWidget<UMAEnchantmentEntryWidget>(this, EntryWidgetClass);
		if (!EntryWidget) continue;

		EntryWidget->SetModule(*Stack->Module, Stack->Count);
		EntryWidget->SetSelected(SelectedRuneEntryId == Entry.EntryId);
		const int32 RuneEntryId = Entry.EntryId;
		EntryWidget->OnSelected.AddWeakLambda(this, [this, RuneEntryId]()
		{
			SelectRune(RuneEntryId);
		});
		RuneContainer->AddChild(EntryWidget);
		RuneEntries.Add(RuneEntryId, EntryWidget);
		bSelectedRuneExists |= SelectedRuneEntryId == RuneEntryId;
	}

	if (!bSelectedRuneExists) SelectedRuneEntryId = INDEX_NONE;
}

void UMAEnchanterWidget::RefreshControls()
{
	TargetContainer->SetIsEnabled(!bRequestPending);
	CompositionWidget->SetIsEnabled(!bRequestPending);
	RuneContainer->SetIsEnabled(!bRequestPending);
	CloseButton->SetIsEnabled(!bRequestPending);
	const bool bCanEnchant = !bRequestPending
		&& EnchanterNPC
		&& SelectedTarget.IsValid()
		&& SelectedEnchantmentSlotIndex != INDEX_NONE
		&& SelectedRuneEntryId != INDEX_NONE;
	EnchantButton->SetIsEnabled(bCanEnchant);
}

void UMAEnchanterWidget::SetSelectedTarget(UMASkillModuleInstance* TargetModule)
{
	if (SelectedTarget.Get() != TargetModule)
	{
		if (SelectedTarget.IsValid() && SubModulesChangedHandle.IsValid())
		{
			SelectedTarget->OnSubModulesChanged.Remove(SubModulesChangedHandle);
		}
		SubModulesChangedHandle.Reset();
		SelectedTarget = TargetModule;
		SelectedEnchantmentSlotIndex = INDEX_NONE;
		if (SelectedTarget.IsValid())
		{
			SubModulesChangedHandle = SelectedTarget->OnSubModulesChanged.AddUObject(
				this,
				&UMAEnchanterWidget::HandleSubModulesChanged);
		}
	}

	for (const TPair<TWeakObjectPtr<UMASkillModuleInstance>, TWeakObjectPtr<UMAEnchantmentEntryWidget>>& Pair : TargetEntries)
	{
		if (Pair.Value.IsValid()) Pair.Value->SetSelected(Pair.Key.Get() == TargetModule);
	}
}

void UMAEnchanterWidget::SelectTarget(UMASkillModuleInstance* TargetModule)
{
	SetSelectedTarget(TargetModule);
	RefreshComposition();
	RefreshControls();
}

void UMAEnchanterWidget::SelectEnchantmentSlot(const int32 SlotIndex)
{
	SelectedEnchantmentSlotIndex = SlotIndex;
	CompositionWidget->SetSelectedSlot(SlotIndex);
	RefreshControls();
}

void UMAEnchanterWidget::SelectRune(const int32 RuneEntryId)
{
	SelectedRuneEntryId = RuneEntryId;
	for (const TPair<int32, TWeakObjectPtr<UMAEnchantmentEntryWidget>>& Pair : RuneEntries)
	{
		if (Pair.Value.IsValid()) Pair.Value->SetSelected(Pair.Key == RuneEntryId);
	}
	RefreshControls();
}

void UMAEnchanterWidget::HandleInventoryChanged()
{
	RefreshRunes();
	RefreshControls();
}

void UMAEnchanterWidget::HandleSkillSlotChanged(FGameplayTag)
{
	RefreshTargets();
	RefreshComposition();
	RefreshControls();
}

void UMAEnchanterWidget::HandleSubModulesChanged(UMASkillModuleInstance*)
{
	SelectedEnchantmentSlotIndex = INDEX_NONE;
	RefreshComposition();
	RefreshControls();
}

void UMAEnchanterWidget::HandleEnchantCompleted(const bool bSucceeded)
{
	bRequestPending = false;
	if (bSucceeded)
	{
		SelectedEnchantmentSlotIndex = INDEX_NONE;
		SelectedRuneEntryId = INDEX_NONE;
	}
	RefreshTargets();
	RefreshComposition();
	RefreshRunes();
	RefreshControls();
}

void UMAEnchanterWidget::HandleEnchantButtonClicked()
{
	bRequestPending = true;
	RefreshControls();
	GetOwningPlayer<AMAPlayerController>()->RequestEnchantModule(
		EnchanterNPC,
		SelectedTarget.Get(),
		SelectedRuneEntryId,
		SelectedEnchantmentSlotIndex);
}

void UMAEnchanterWidget::HandleCloseButtonClicked()
{
	if (bRequestPending) return;

	if (EnchanterNPC)
	{
		EnchanterNPC->CloseEnchanter();
	}
}
