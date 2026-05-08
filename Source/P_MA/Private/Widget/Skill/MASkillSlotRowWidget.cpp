#include "Widget/Skill/MASkillSlotRowWidget.h"

#include "Components/HorizontalBox.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "Input/MAInputStatics.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/MAPlayerController.h"
#include "Widget/Skill/MASkillIconWidget.h"
#include "Widget/Skill/MASkillModuleSocketWidget.h"

void UMASkillSlotRowWidget::InitializeSlot(UMASkillManagerComponent* InSkillManager, EMAAbilityInputID InInputID)
{
	if (SkillManager)
	{
		SkillManager->OnSkillSlotChanged.RemoveAll(this);
	}
	if (InputBindingsOwner.IsValid())
	{
		InputBindingsOwner->OnInputBindingsChanged.RemoveAll(this);
	}

	SkillManager = InSkillManager;
	InputID = InInputID;

	if (SkillManager)
	{
		SkillManager->OnSkillSlotChanged.AddUObject(this, &UMASkillSlotRowWidget::HandleSkillSlotChanged);
	}
	InputBindingsOwner = GetOwningPlayer<AMAPlayerController>();
	if (InputBindingsOwner.IsValid())
	{
		InputBindingsOwner->OnInputBindingsChanged.AddUObject(this, &UMASkillSlotRowWidget::RefreshHotkeyText);
	}

	Refresh();
	RefreshHotkeyText();
}

void UMASkillSlotRowWidget::NativeDestruct()
{
	if (SkillManager)
	{
		SkillManager->OnSkillSlotChanged.RemoveAll(this);
	}
	if (InputBindingsOwner.IsValid())
	{
		InputBindingsOwner->OnInputBindingsChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UMASkillSlotRowWidget::Refresh()
{
	const TArray<UMASkillDefinition*> Definitions = SkillManager
		? SkillManager->GetDefinitions(InputID)
		: TArray<UMASkillDefinition*>();

	RebuildModuleSockets(Definitions);
}

void UMASkillSlotRowWidget::HandleSkillSlotChanged(EMAAbilityInputID ChangedInputID)
{
	if (ChangedInputID != InputID) return;

	Refresh();
}

void UMASkillSlotRowWidget::RefreshHotkeyText()
{
	if (!SkillIconWidget) return;

	const APlayerController* PlayerController = GetOwningPlayer();
	const AMAPlayerCharacter* PlayerCharacter = PlayerController
		? Cast<AMAPlayerCharacter>(PlayerController->GetPawn())
		: Cast<AMAPlayerCharacter>(GetOwningPlayerPawn());

	SkillIconWidget->SetHotkeyText(FMAInputStatics::GetGameplayAbilityInputText(PlayerController, PlayerCharacter, InputID));
}

void UMASkillSlotRowWidget::RebuildModuleSockets(const TArray<UMASkillDefinition*>& InSkillDefinitions)
{
	if (!ModuleSocketBox) return;

	ModuleSocketBox->ClearChildren();
	if (!ModuleSocketWidgetClass) return;

	for (int32 Index = 0; Index < InSkillDefinitions.Num(); ++Index)
	{
		UMASkillModuleSocketWidget* SocketWidget = CreateWidget<UMASkillModuleSocketWidget>(this, ModuleSocketWidgetClass);
		if (!SocketWidget) continue;

		SocketWidget->InitializeSocket(SkillManager, InputID, Index, InSkillDefinitions[Index]);
		ModuleSocketBox->AddChildToHorizontalBox(SocketWidget);
	}
}
