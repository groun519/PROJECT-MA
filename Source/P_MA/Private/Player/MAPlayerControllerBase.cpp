// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/MAPlayerControllerBase.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Widget/Settings/SettingsWidget.h"
#include "Widget/System/SystemMenuWidget.h"

void AMAPlayerControllerBase::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (SystemMenuInputMapping)
			{
				InputSubsystem->RemoveMappingContext(SystemMenuInputMapping);
				InputSubsystem->AddMappingContext(SystemMenuInputMapping, 1);
			}
		}
	}

	if (SystemMenuToggleInputAction)
	{
		if (UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(InputComponent))
		{
			EnhancedInputComp->BindAction(SystemMenuToggleInputAction, ETriggerEvent::Started, this, &AMAPlayerControllerBase::ToggleSystemMenu);
		}
	}
}

void AMAPlayerControllerBase::ToggleSystemMenu()
{
	if (!IsLocalController()) return;

	if (ActiveSettingsWidget && ActiveSettingsWidget->IsInViewport())
	{
		CloseSettingsWidget();
		return;
	}

	if (ActiveSystemMenuWidget && ActiveSystemMenuWidget->IsInViewport())
	{
		CloseSystemMenu();
		return;
	}

	if (!SystemMenuWidgetClass) return;

	ActiveSystemMenuWidget = CreateWidget<USystemMenuWidget>(this, SystemMenuWidgetClass);
	if (!ActiveSystemMenuWidget) return;

	ActiveSystemMenuWidget->OnActionRequested.RemoveAll(this);
	ActiveSystemMenuWidget->OnActionRequested.AddUObject(this, &AMAPlayerControllerBase::HandleSystemMenuActionRequested);
	ActiveSystemMenuWidget->AddToViewport(200);

	ApplySystemMenuOpenInputMode();
}

void AMAPlayerControllerBase::ApplySystemMenuOpenInputMode()
{
	ApplyWidgetFocusInputMode(ActiveSystemMenuWidget);
}

void AMAPlayerControllerBase::CloseSystemMenu()
{
	if (ActiveSystemMenuWidget)
	{
		ActiveSystemMenuWidget->RemoveFromParent();
		ActiveSystemMenuWidget = nullptr;
	}

	ApplySystemMenuClosedInputMode();
}

void AMAPlayerControllerBase::CloseSettingsWidget()
{
	if (!ActiveSettingsWidget) return;

	ActiveSettingsWidget->RemoveFromParent();
	ActiveSettingsWidget = nullptr;
	ApplyGameAndUiInputMode();
}

void AMAPlayerControllerBase::RefreshSettingsFocus()
{
	if (ActiveSettingsWidget && ActiveSettingsWidget->IsInViewport())
	{
		ApplyWidgetFocusInputMode(ActiveSettingsWidget);
		return;
	}

	ApplyGameAndUiInputMode();
}

void AMAPlayerControllerBase::ApplySystemMenuClosedInputMode()
{
	ApplyGameAndUiInputMode();
}

void AMAPlayerControllerBase::ApplyWidgetFocusInputMode(UUserWidget* TargetWidget)
{
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(TargetWidget->TakeWidget());
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AMAPlayerControllerBase::ApplyGameAndUiInputMode()
{
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AMAPlayerControllerBase::HandleSystemMenuActionRequested(ESystemMenuAction Action)
{
	switch (Action)
	{
	case ESystemMenuAction::Close:
		CloseSystemMenu();
		break;
	case ESystemMenuAction::Settings:
		CloseSystemMenu();
		OpenSettingsWidget();
		break;
	case ESystemMenuAction::Exit:
		UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
		break;
	default:
		break;
	}
}

void AMAPlayerControllerBase::OpenSettingsWidget()
{
	if (ActiveSettingsWidget && ActiveSettingsWidget->IsInViewport())
	{
		ApplyWidgetFocusInputMode(ActiveSettingsWidget);
		return;
	}

	if (!SettingsWidgetClass) return;

	ActiveSettingsWidget = CreateWidget<USettingsWidget>(this, SettingsWidgetClass);
	if (!ActiveSettingsWidget) return;

	ActiveSettingsWidget->AddToViewport(210);
	ApplyWidgetFocusInputMode(ActiveSettingsWidget);
}
