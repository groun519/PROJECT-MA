#include "Widget/Input/MAInputKeyPromptWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Input/MAInputStatics.h"
#include "Player/MAPlayerController.h"

void UMAInputKeyPromptWidget::NativeDestruct()
{
	ClearInputAction();
	Super::NativeDestruct();
}

void UMAInputKeyPromptWidget::SetInputAction(APlayerController* PlayerController, UInputMappingContext* MappingContext, UInputAction* InputAction)
{
	UnbindInputBindingChanges();

	InputOwner = PlayerController;
	InputMappingContext = MappingContext;
	BoundInputAction = InputAction;

	if (AMAPlayerController* Player = Cast<AMAPlayerController>(PlayerController))
	{
		Player->OnInputBindingsChanged.AddUObject(this, &UMAInputKeyPromptWidget::RefreshKeyText);
	}

	RefreshKeyText();
}

void UMAInputKeyPromptWidget::ClearInputAction()
{
	UnbindInputBindingChanges();

	InputOwner.Reset();
	InputMappingContext.Reset();
	BoundInputAction.Reset();
	KeyText->SetText(FText::GetEmpty());
}

void UMAInputKeyPromptWidget::RefreshKeyText()
{
	KeyText->SetText(FMAInputStatics::GetInputActionText(
		InputOwner.Get(),
		InputMappingContext.Get(),
		BoundInputAction.Get()));
}

void UMAInputKeyPromptWidget::UnbindInputBindingChanges()
{
	if (AMAPlayerController* Player = Cast<AMAPlayerController>(InputOwner.Get()))
	{
		Player->OnInputBindingsChanged.RemoveAll(this);
	}
}
