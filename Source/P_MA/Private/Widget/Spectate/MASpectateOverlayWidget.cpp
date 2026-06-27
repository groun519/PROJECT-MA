#include "Widget/Spectate/MASpectateOverlayWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
#include "Player/MAPlayerCharacter.h"
#include "Widget/Input/MAInputKeyPromptWidget.h"

void UMASpectateOverlayWidget::NativeDestruct()
{
	if (LeftKeyPrompt)
	{
		LeftKeyPrompt->ClearInputContext();
	}
	if (RightKeyPrompt)
	{
		RightKeyPrompt->ClearInputContext();
	}

	Super::NativeDestruct();
}

void UMASpectateOverlayWidget::InitializeSpectateOverlay(APlayerController* PlayerController, UInputMappingContext* MappingContext)
{
	if (LeftKeyPrompt)
	{
		LeftKeyPrompt->SetInputContext(PlayerController, MappingContext);
	}
	if (RightKeyPrompt)
	{
		RightKeyPrompt->SetInputContext(PlayerController, MappingContext);
	}
}

void UMASpectateOverlayWidget::SetSpectateTarget(AMAPlayerCharacter* SpectateTarget, int32 TargetCount)
{
	if (!TargetNameText) return;

	FText TargetText = NSLOCTEXT("Spectate", "NoSpectateTarget", "No spectate target");
	if (SpectateTarget)
	{
		if (const APlayerState* PlayerState = SpectateTarget->GetPlayerState())
		{
			TargetText = FText::FromString(PlayerState->GetPlayerName());
		}
		else
		{
			TargetText = FText::FromString(SpectateTarget->GetName());
		}
	}

	if (TargetCount > 1)
	{
		TargetText = FText::Format(NSLOCTEXT("Spectate", "SpectateTargetWithCount", "{0} ({1})"), TargetText, TargetCount);
	}

	TargetNameText->SetText(TargetText);
}
