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

	void SetInputContext(APlayerController* PlayerController, UInputMappingContext* MappingContext);
	void ClearInputContext();

private:
	void RefreshKeyText();
	void UnbindInputBindingChanges();

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> KeyText;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> InputAction;

	TWeakObjectPtr<APlayerController> InputOwner;
	TWeakObjectPtr<UInputMappingContext> InputMappingContext;
};
