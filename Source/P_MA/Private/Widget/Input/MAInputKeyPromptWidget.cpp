#include "Widget/Input/MAInputKeyPromptWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Input/MAInputStatics.h"
#include "InputMappingContext.h"
#include "Player/MAPlayerController.h"

void UMAInputKeyPromptWidget::NativeDestruct()
{
	ClearInputContext();
	Super::NativeDestruct();
}

void UMAInputKeyPromptWidget::SetInputContext(APlayerController* PlayerController, UInputMappingContext* MappingContext)
{
	check(InputAction);
	UnbindInputBindingChanges();

	InputOwner = PlayerController;
	InputMappingContext = MappingContext;

	if (AMAPlayerController* Player = Cast<AMAPlayerController>(PlayerController))
	{
		Player->OnInputBindingsChanged.AddUObject(this, &UMAInputKeyPromptWidget::RefreshKeyText);
	}

	RefreshKeyText();
}

void UMAInputKeyPromptWidget::ClearInputContext()
{
	UnbindInputBindingChanges();

	InputOwner.Reset();
	InputMappingContext.Reset();
	KeyText->SetText(FText::GetEmpty());
}

void UMAInputKeyPromptWidget::RefreshKeyText()
{
	KeyText->SetText(FMAInputStatics::GetInputActionText(
		InputOwner.Get(),
		InputMappingContext.Get(),
		InputAction));
}

void UMAInputKeyPromptWidget::UnbindInputBindingChanges()
{
	if (AMAPlayerController* Player = Cast<AMAPlayerController>(InputOwner.Get()))
	{
		Player->OnInputBindingsChanged.RemoveAll(this);
	}
}
