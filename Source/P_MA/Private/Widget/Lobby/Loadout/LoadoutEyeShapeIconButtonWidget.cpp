// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutEyeShapeIconButtonWidget.h"

void ULoadoutEyeShapeIconButtonWidget::OnButtonClicked()
{
	Super::OnButtonClicked();
	OnEyeShapeSelected.Broadcast(EyeShapeId);
}
