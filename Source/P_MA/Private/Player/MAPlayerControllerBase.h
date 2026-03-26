// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MAPlayerControllerBase.generated.h"

class UInputAction;
class UInputMappingContext;
class USettingsWidget;
class USystemMenuWidget;
class UUserWidget;
enum class ESettingsCategory : uint8;
enum class ESystemMenuAction : uint8;

UCLASS()
class P_MA_API AMAPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleSystemMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseSystemMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseSettingsWidget();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void RefreshSettingsFocus();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ReopenSettingsWidget();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* SystemMenuInputMapping;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* SystemMenuToggleInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USystemMenuWidget> SystemMenuWidgetClass;

	UPROPERTY()
	TObjectPtr<USystemMenuWidget> ActiveSystemMenuWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USettingsWidget> SettingsWidgetClass;

	UPROPERTY()
	TObjectPtr<USettingsWidget> ActiveSettingsWidget;

	virtual void ApplySystemMenuOpenInputMode();
	virtual void ApplySystemMenuClosedInputMode();
	void ApplyWidgetFocusInputMode(UUserWidget* TargetWidget);
	void ApplyGameAndUiInputMode();

private:
	void HandleSystemMenuActionRequested(ESystemMenuAction Action);
	void OpenSettingsWidget(ESettingsCategory InitialCategory);
};
