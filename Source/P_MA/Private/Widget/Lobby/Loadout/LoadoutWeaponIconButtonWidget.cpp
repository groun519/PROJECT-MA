// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutWeaponIconButtonWidget.h"

void ULoadoutWeaponIconButtonWidget::OnButtonClicked()
{
	Super::OnButtonClicked();
	OnWeaponSelected.Broadcast(WeaponId);
}

