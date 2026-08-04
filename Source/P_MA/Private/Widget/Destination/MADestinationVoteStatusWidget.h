#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MADestinationVoteStatusWidget.generated.h"

class APlayerState;
class UHorizontalBox;
class UMADestinationVoterIconWidget;

UCLASS()
class P_MA_API UMADestinationVoteStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetVoters(const TArray<TObjectPtr<APlayerState>>& Voters);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> VoterIconBox;

	UPROPERTY(EditDefaultsOnly, Category="Destination|Vote")
	TSubclassOf<UMADestinationVoterIconWidget> VoterIconWidgetClass;
};
