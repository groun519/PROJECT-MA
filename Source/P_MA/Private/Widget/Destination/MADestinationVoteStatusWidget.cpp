#include "Widget/Destination/MADestinationVoteStatusWidget.h"

#include "Components/HorizontalBox.h"
#include "Player/MAPlayerState.h"
#include "Widget/Destination/MADestinationVoterIconWidget.h"

void UMADestinationVoteStatusWidget::SetVoters(const TArray<TObjectPtr<APlayerState>>& Voters)
{
	VoterIconBox->ClearChildren();
	if (!VoterIconWidgetClass) return;

	for (APlayerState* Voter : Voters)
	{
		const AMAPlayerState* MAPlayerState = Cast<AMAPlayerState>(Voter);
		if (!MAPlayerState) continue;

		UMADestinationVoterIconWidget* VoterIcon = CreateWidget<UMADestinationVoterIconWidget>(this, VoterIconWidgetClass);
		if (!VoterIcon) continue;

		const FMaterialParamDataPair Colors = MAPlayerState->GetLoadoutColor();
		VoterIcon->SetVoterColors(Colors.BodyData.Color, Colors.EyeData.Color);
		VoterIconBox->AddChild(VoterIcon);
	}
}
