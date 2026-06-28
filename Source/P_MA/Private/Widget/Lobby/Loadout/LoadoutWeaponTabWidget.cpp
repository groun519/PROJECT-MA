// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutWeaponTabWidget.h"
#include "Components/PanelWidget.h"
#include "Components/ScrollBox.h"
#include "Engine/DataTable.h"
#include "Framework/MAGameInstance.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "Level/Lobby/LobbyPlayerController.h"
#include "Player/Loadout/Data/LoadoutDataSet.h"
#include "Player/Loadout/Data/LoadoutWeaponData.h"
#include "Widget/Lobby/Loadout/LoadoutWeaponIconButtonWidget.h"
#include "Widget/Lobby/Loadout/LoadoutWeaponModuleButtonWidget.h"
#include "Widget/Skill/MASkillTooltipWidget.h"

void ULoadoutWeaponTabWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const UMAGameInstance* GI = GetGameInstance<UMAGameInstance>();
	const ULoadoutDataSet* LoadoutDataSet = GI ? GI->TryGetLoadoutDataSet() : nullptr;
	WeaponDataTable = LoadoutDataSet ? LoadoutDataSet->WeaponDataTable : nullptr;

	BuildWeaponButtons();
}

void ULoadoutWeaponTabWidget::BuildWeaponButtons()
{
	if (!WeaponScrollBox || !WeaponButtonClass || !WeaponDataTable)
	{
		return;
	}

	WeaponScrollBox->ClearChildren();
	WeaponButtons.Reset();

	const TArray<FName> RowNames = WeaponDataTable->GetRowNames();

	for (const FName RowName : RowNames)
	{
		const FLoadoutWeaponDataRow* Row = FindWeaponData(RowName);
		if (!Row)
		{
			continue;
		}

		ULoadoutWeaponIconButtonWidget* ButtonWidget = CreateWidget<ULoadoutWeaponIconButtonWidget>(this, WeaponButtonClass);
		if (!ButtonWidget)
		{
			continue;
		}

		ButtonWidget->WeaponId = RowName;
		ButtonWidget->IconTexture = Row->IconTexture;
		ButtonWidget->OnWeaponSelected.AddDynamic(this, &ULoadoutWeaponTabWidget::HandleWeaponSelected);
		AddButtonToScrollBox(WeaponScrollBox, ButtonWidget);

		WeaponButtons.Add(ButtonWidget);
	}
}

void ULoadoutWeaponTabWidget::HandleWeaponSelected(FName WeaponId)
{
	if (ALobbyPlayerController* PC = GetOwningPlayer<ALobbyPlayerController>())
	{
		const FLoadoutWeaponDataRow* Row = FindWeaponData(WeaponId);
		if (!Row)
		{
			return;
		}

		USkeletalMesh* Mesh = Row->WeaponMesh.LoadSynchronous();
		PC->PreviewWeapon(WeaponId, Mesh, Row->WeaponOffset);
		UpdateSelectedWeapon(WeaponId);
		RefreshProvidedModules(Row);
	}
}

void ULoadoutWeaponTabWidget::SyncFromPendingWeapon(FName WeaponId)
{
	UpdateSelectedWeapon(WeaponId);
	RefreshProvidedModules(FindWeaponData(WeaponId));
}

void ULoadoutWeaponTabWidget::UpdateSelectedWeapon(FName WeaponId)
{
	for (ULoadoutWeaponIconButtonWidget* Button : WeaponButtons)
	{
		if (!Button)
		{
			continue;
		}
		Button->SetSelected(Button->WeaponId == WeaponId);
	}
}

const FLoadoutWeaponDataRow* ULoadoutWeaponTabWidget::FindWeaponData(FName WeaponId) const
{
	return WeaponDataTable && !WeaponId.IsNone()
		? WeaponDataTable->FindRow<FLoadoutWeaponDataRow>(WeaponId, TEXT("LoadoutWeaponTab"))
		: nullptr;
}

void ULoadoutWeaponTabWidget::RefreshProvidedModules(const FLoadoutWeaponDataRow* WeaponData)
{
	SelectedModuleButton = nullptr;
	ProvidedModulePanel->ClearChildren();
	ModuleDetailWidget->SetVisibility(ESlateVisibility::Collapsed);

	if (!WeaponData || !ModuleButtonClass) return;

	TArray<UMASkillDefinition*> ProvidedModules;
	if (UMASkillDefinition* AttackSkillDefinition = WeaponData->AttackSkillDefinition.LoadSynchronous())
	{
		ProvidedModules.Add(AttackSkillDefinition);
	}

	ULoadoutWeaponModuleButtonWidget* DefaultModuleButton = nullptr;
	for (UMASkillDefinition* ModuleDefinition : ProvidedModules)
	{
		ULoadoutWeaponModuleButtonWidget* ModuleButton =
			CreateWidget<ULoadoutWeaponModuleButtonWidget>(this, ModuleButtonClass);
		if (!ModuleButton) continue;

		ModuleButton->SetModuleDefinition(ModuleDefinition);
		ModuleButton->OnModuleSelected.AddUObject(this, &ULoadoutWeaponTabWidget::HandleProvidedModuleSelected);
		ProvidedModulePanel->AddChild(ModuleButton);
		if (!DefaultModuleButton) DefaultModuleButton = ModuleButton;
	}

	SelectProvidedModule(DefaultModuleButton);
}

void ULoadoutWeaponTabWidget::HandleProvidedModuleSelected(ULoadoutWeaponModuleButtonWidget* ModuleButton)
{
	SelectProvidedModule(ModuleButton);
}

void ULoadoutWeaponTabWidget::SelectProvidedModule(ULoadoutWeaponModuleButtonWidget* ModuleButton)
{
	if (SelectedModuleButton)
	{
		SelectedModuleButton->SetSelected(false);
	}

	SelectedModuleButton = ModuleButton;
	if (SelectedModuleButton)
	{
		SelectedModuleButton->SetSelected(true);
	}

	UMASkillDefinition* ModuleDefinition = ModuleButton ? ModuleButton->GetModuleDefinition() : nullptr;
	if (ModuleDefinition)
	{
		constexpr bool bShowTagsAndMessages = false;
		ModuleDetailWidget->SetSkillTooltip(ModuleDefinition, FGameplayTag(), nullptr, bShowTagsAndMessages);
	}
	ModuleDetailWidget->SetVisibility(ModuleDefinition
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
}
