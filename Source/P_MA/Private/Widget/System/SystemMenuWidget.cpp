// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/System/SystemMenuWidget.h"

#include "Components/Button.h"

void USystemMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CloseButton->OnClicked.AddUniqueDynamic(this, &USystemMenuWidget::HandleCloseButtonClicked);
	SettingsButton->OnClicked.AddUniqueDynamic(this, &USystemMenuWidget::HandleSettingsButtonClicked);
	ExitButton->OnClicked.AddUniqueDynamic(this, &USystemMenuWidget::HandleExitButtonClicked);
}

void USystemMenuWidget::HandleCloseButtonClicked()
{
	OnActionRequested.Broadcast(ESystemMenuAction::Close);
}

void USystemMenuWidget::HandleSettingsButtonClicked()
{
	OnActionRequested.Broadcast(ESystemMenuAction::Settings);
}

void USystemMenuWidget::HandleExitButtonClicked()
{
	OnActionRequested.Broadcast(ESystemMenuAction::Exit);
}
