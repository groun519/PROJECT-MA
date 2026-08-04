#include "Widget/GameOver/MAGameOverWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UMAGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);

	ReturnToLobbyButton->OnClicked.AddUniqueDynamic(this, &UMAGameOverWidget::HandleReturnToLobbyClicked);
	SettingsButton->OnClicked.AddUniqueDynamic(this, &UMAGameOverWidget::HandleSettingsClicked);
	ExitButton->OnClicked.AddUniqueDynamic(this, &UMAGameOverWidget::HandleExitClicked);
}

void UMAGameOverWidget::SetHostControls(bool bIsHost)
{
	ReturnToLobbyButton->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	WaitingHostText->SetVisibility(bIsHost ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
}

void UMAGameOverWidget::HandleReturnToLobbyClicked()
{
	OnActionRequested.Broadcast(EMAGameOverAction::ReturnToLobby);
}

void UMAGameOverWidget::HandleSettingsClicked()
{
	OnActionRequested.Broadcast(EMAGameOverAction::Settings);
}

void UMAGameOverWidget::HandleExitClicked()
{
	OnActionRequested.Broadcast(EMAGameOverAction::Exit);
}
