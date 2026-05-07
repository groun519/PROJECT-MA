#include "Widget/Skill/MASkillSlotRowWidget.h"

#include "Components/HorizontalBox.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "Widget/Skill/MASkillModuleSocketWidget.h"

void UMASkillSlotRowWidget::InitializeSlot(UMASkillManagerComponent* InSkillManager, EMAAbilityInputID InInputID)
{
	if (SkillManager)
	{
		SkillManager->OnSkillSlotChanged.RemoveAll(this);
	}

	SkillManager = InSkillManager;
	InputID = InInputID;

	if (SkillManager)
	{
		SkillManager->OnSkillSlotChanged.AddUObject(this, &UMASkillSlotRowWidget::HandleSkillSlotChanged);
	}

	Refresh();
}

void UMASkillSlotRowWidget::NativeDestruct()
{
	if (SkillManager)
	{
		SkillManager->OnSkillSlotChanged.RemoveAll(this);
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
