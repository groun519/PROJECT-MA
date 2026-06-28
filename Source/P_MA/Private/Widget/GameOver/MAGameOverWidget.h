#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MAGameOverWidget.generated.h"

class UButton;
class UTextBlock;

UENUM()
enum class EMAGameOverAction : uint8
{
	ReturnToLobby,
	Settings,
	Exit
};

DECLARE_MULTICAST_DELEGATE_OneParam(FMAGameOverActionRequested, EMAGameOverAction);

UCLASS()
class P_MA_API UMAGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetHostControls(bool bIsHost);

	FMAGameOverActionRequested OnActionRequested;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ReturnToLobbyButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> SettingsButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ExitButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> WaitingHostText;

private:
	UFUNCTION()
	void HandleReturnToLobbyClicked();

	UFUNCTION()
	void HandleSettingsClicked();

	UFUNCTION()
	void HandleExitClicked();
};
