// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutMountIconButtonWidget.h"

void ULoadoutMountIconButtonWidget::OnButtonClicked()
{
	Super::OnButtonClicked();
	OnMountSelected.Broadcast(MountId);
}
