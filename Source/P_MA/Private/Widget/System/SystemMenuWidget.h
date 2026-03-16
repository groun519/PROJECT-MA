// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SystemMenuWidget.generated.h"

class UButton;

UENUM()
enum class ESystemMenuAction : uint8
{
	Close,
	Settings,
	Exit
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSystemMenuActionRequested, ESystemMenuAction);

UCLASS()
class P_MA_API USystemMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	FOnSystemMenuActionRequested OnActionRequested;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SettingsButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

private:
	UFUNCTION()
	void HandleCloseButtonClicked();

	UFUNCTION()
	void HandleSettingsButtonClicked();

	UFUNCTION()
	void HandleExitButtonClicked();
};
