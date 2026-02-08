// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutWeaponTabWidget.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Engine/DataTable.h"
#include "Player/Loadout/Data/LoadoutWeaponData.h"
#include "Widget/Lobby/Loadout/LoadoutWeaponButtonWidget.h"
#include "Level/Lobby/LobbyPlayerController.h"
#include "Level/Lobby/LobbyGameState.h"
#include "Level/Lobby/LobbyAvatarSlot.h"
#include "GameFramework/PlayerState.h"
#include "Player/MAPlayerState.h"

void ULoadoutWeaponTabWidget::NativeConstruct()
{
	Super::NativeConstruct();

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

	TArray<FName> RowNames = WeaponDataTable->GetRowNames();
	RowNames.Sort(FNameLexicalLess());

	for (const FName RowName : RowNames)
	{
		const FLoadoutWeaponDataRow* Row = WeaponDataTable->FindRow<FLoadoutWeaponDataRow>(RowName, TEXT("LoadoutWeaponTab"));
		if (!Row)
		{
			continue;
		}

		ULoadoutWeaponButtonWidget* ButtonWidget = CreateWidget<ULoadoutWeaponButtonWidget>(this, WeaponButtonClass);
		if (!ButtonWidget)
		{
			continue;
		}

		ButtonWidget->WeaponId = RowName;
		ButtonWidget->WeaponName = Row->WeaponName;
		ButtonWidget->IconTexture = Row->IconTexture;
		ButtonWidget->OnWeaponSelected.AddDynamic(this, &ULoadoutWeaponTabWidget::HandleWeaponSelected);

		if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(WeaponScrollBox->AddChild(ButtonWidget)))
		{
			ScrollSlot->SetPadding(FMargin(6.f, 0.f, 6.f, 0.f));
		}

		WeaponButtons.Add(ButtonWidget);
	}

	if (ALobbyPlayerController* PC = GetOwningPlayer<ALobbyPlayerController>())
	{
		if (AMAPlayerState* PS = PC->GetPlayerState<AMAPlayerState>())
		{
			UpdateSelectedWeapon(PS->GetLoadoutWeaponId());
		}
	}
}

void ULoadoutWeaponTabWidget::HandleWeaponSelected(FName WeaponId)
{
	if (ALobbyPlayerController* PC = GetOwningPlayer<ALobbyPlayerController>())
	{
		if (!WeaponDataTable)
		{
			return;
		}

		const FLoadoutWeaponDataRow* Row = WeaponDataTable->FindRow<FLoadoutWeaponDataRow>(WeaponId, TEXT("LoadoutWeaponTab"));
		if (!Row)
		{
			return;
		}

		USkeletalMesh* Mesh = Row->WeaponMesh.LoadSynchronous();
		PC->PreviewWeapon(WeaponId, Mesh, Row->WeaponOffset);
		UpdateSelectedWeapon(WeaponId);
	}
}

void ULoadoutWeaponTabWidget::UpdateSelectedWeapon(FName WeaponId)
{
	for (ULoadoutWeaponButtonWidget* Button : WeaponButtons)
	{
		if (!Button)
		{
			continue;
		}
		Button->SetSelected(Button->WeaponId == WeaponId);
	}
}
