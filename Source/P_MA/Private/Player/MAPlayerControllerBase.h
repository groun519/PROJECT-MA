#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MAPlayerControllerBase.generated.h"

class UInputAction;
class UInputMappingContext;
class UMACameraOcclusionCutoutComponent;
class UMAPlayerCameraDirectorComponent;
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
	AMAPlayerControllerBase();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void AcknowledgePossession(APawn* P) override;
	virtual void SetupInputComponent() override;

	UFUNCTION(Server, Reliable)
	void ServerNotifyLoaded();

	UMAPlayerCameraDirectorComponent* GetCameraDirector() const { return CameraDirectorComponent; }
	UMACameraOcclusionCutoutComponent* GetCameraOcclusionCutout() const { return CameraOcclusionCutoutComponent; }

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

	void OpenSettings(ESettingsCategory InitialCategory);
	void ApplyWidgetFocusInputMode(UUserWidget* TargetWidget);
	void ApplyGameAndUiInputMode();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* SystemMenuInputMapping;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* SystemMenuToggleInputAction;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UMAPlayerCameraDirectorComponent> CameraDirectorComponent;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UMACameraOcclusionCutoutComponent> CameraOcclusionCutoutComponent;

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

private:
	void HandleSystemMenuActionRequested(ESystemMenuAction Action);
	void OpenSettingsWidget(ESettingsCategory InitialCategory);
};
