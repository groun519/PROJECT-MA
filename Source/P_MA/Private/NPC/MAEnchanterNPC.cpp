#include "NPC/MAEnchanterNPC.h"

#include "Convenience/MAInteractableComponent.h"
#include "Convenience/MAInteractorComponent.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "Inventory/MAInventoryComponent.h"
#include "Player/MAPlayerCharacter.h"
#include "Setting/MAGameSettings.h"
#include "Widget/Enchantment/MAEnchanterWidget.h"

AMAEnchanterNPC::AMAEnchanterNPC()
{
	InteractableComponent->CALL_SETUP_INTERACT(HandleInteract);
}

void AMAEnchanterNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CloseEnchanter();
	Super::EndPlay(EndPlayReason);
}

bool AMAEnchanterNPC::EnchantModule(
	APlayerController* PlayerController,
	UMASkillModuleInstance* TargetModule,
	const int32 RuneEntryId,
	const int32 EnchantmentSlotIndex)
{
	if (!HasAuthority()) return false;

	AMAPlayerCharacter* PlayerCharacter = PlayerController
		? Cast<AMAPlayerCharacter>(PlayerController->GetPawn())
		: nullptr;
	UMAInventoryComponent* Inventory = PlayerCharacter
		? PlayerCharacter->GetInventoryComponent()
		: nullptr;
	if (!PlayerCharacter || !Inventory || !InteractableComponent->IsInteractorInRange(PlayerCharacter))
	{
		return false;
	}
	if (!TargetModule
		|| !TargetModule->IsValid()
		|| TargetModule->GetTypedOuter<AActor>() != PlayerCharacter
		|| !TargetModule->IsInSkillSlot()
		|| EnchantmentSlotIndex < 0
		|| EnchantmentSlotIndex >= GetEnchantSlotCount())
	{
		return false;
	}

	const FMAInventoryEntry* RuneEntry = Inventory->FindEntry(RuneEntryId);
	const FMAInventoryStack* RuneStack = RuneEntry ? RuneEntry->GetStack() : nullptr;
	UMASkillModule* RuneModule = RuneStack ? RuneStack->Module.Get() : nullptr;
	if (!RuneModule || RuneModule->GetModuleType() != EMASkillModuleType::Sub)
	{
		return false;
	}

	const int32 RemainingRuneCount = RuneStack->Count - 1;
	if (!TargetModule->SetSubModuleAt(EnchantmentSlotIndex, RuneModule))
	{
		return false;
	}

	verify(Inventory->SetStackCount(RuneEntryId, RemainingRuneCount));
	return true;
}

int32 AMAEnchanterNPC::GetEnchantSlotCount() const
{
	return FMath::Max(0, UMAGameSettings::Get()->MaxEnchantmentsPerModule);
}

void AMAEnchanterNPC::CloseEnchanter()
{
	APlayerController* PlayerController = ActiveEnchanterWidget
		? ActiveEnchanterWidget->GetOwningPlayer()
		: nullptr;

	if (ActiveEnchanterWidget)
	{
		ActiveEnchanterWidget->RemoveFromParent();
		ActiveEnchanterWidget = nullptr;
	}

	if (ActiveInteractor.IsValid())
	{
		AMAPlayerCharacter* Interactor = ActiveInteractor.Get();
		Interactor->GetInteractorComponent()->SetInteractionEnabled(true, Interactor);
		ActiveInteractor.Reset();
	}
	if (!PlayerController) return;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
}

void AMAEnchanterNPC::HandleInteract(AMAPlayerCharacter* Interactor)
{
	if (!Interactor || !EnchanterWidgetClass) return;
	if (ActiveEnchanterWidget && ActiveEnchanterWidget->IsInViewport()) return;

	APlayerController* PlayerController = Cast<APlayerController>(Interactor->GetController());
	if (!PlayerController || !PlayerController->IsLocalController()) return;

	if (PlayerController->PlayerInput)
	{
		PlayerController->PlayerInput->FlushPressedKeys();
	}

	ActiveEnchanterWidget = CreateWidget<UMAEnchanterWidget>(PlayerController, EnchanterWidgetClass);
	if (!ActiveEnchanterWidget) return;

	Interactor->GetInteractorComponent()->SetInteractionEnabled(false, Interactor);
	ActiveInteractor = Interactor;

	ActiveEnchanterWidget->AddToViewport(100);
	ActiveEnchanterWidget->InitializeEnchanter(this);
	ActiveEnchanterWidget->SetKeyboardFocus();

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
}
