#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MASpectateOverlayWidget.generated.h"

class APlayerController;
class AMAPlayerCharacter;
class UInputMappingContext;
class UMAInputKeyPromptWidget;
class UTextBlock;

UCLASS()
class P_MA_API UMASpectateOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeDestruct() override;

	void InitializeSpectateOverlay(APlayerController* PlayerController, UInputMappingContext* MappingContext);
	void SetSpectateTarget(AMAPlayerCharacter* SpectateTarget, int32 TargetCount);

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TargetNameText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMAInputKeyPromptWidget> LeftKeyPrompt;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMAInputKeyPromptWidget> RightKeyPrompt;
};
