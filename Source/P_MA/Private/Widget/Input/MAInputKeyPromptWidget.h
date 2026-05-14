#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MAInputKeyPromptWidget.generated.h"

class APlayerController;
class UInputAction;
class UInputMappingContext;
class UTextBlock;

UCLASS()
class P_MA_API UMAInputKeyPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeDestruct() override;

	void SetInputAction(APlayerController* PlayerController, UInputMappingContext* MappingContext, UInputAction* InputAction);
	void ClearInputAction();

private:
	void RefreshKeyText();
	void UnbindInputBindingChanges();

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> KeyText;

	TWeakObjectPtr<APlayerController> InputOwner;
	TWeakObjectPtr<UInputMappingContext> InputMappingContext;
	TWeakObjectPtr<UInputAction> BoundInputAction;
};
