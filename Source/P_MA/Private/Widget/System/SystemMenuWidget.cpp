// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/System/SystemMenuWidget.h"

#include "Components/Button.h"
#include "InputCoreTypes.h"
#include "Player/MAPlayerControllerBase.h"

void USystemMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);

	CloseButton->OnClicked.AddUniqueDynamic(this, &USystemMenuWidget::HandleCloseButtonClicked);
	SettingsButton->OnClicked.AddUniqueDynamic(this, &USystemMenuWidget::HandleSettingsButtonClicked);
	ExitButton->OnClicked.AddUniqueDynamic(this, &USystemMenuWidget::HandleExitButtonClicked);
}

FReply USystemMenuWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		if (AMAPlayerControllerBase* MAPlayerController = GetOwningPlayer<AMAPlayerControllerBase>()) MAPlayerController->CloseSystemMenu();
		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
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
