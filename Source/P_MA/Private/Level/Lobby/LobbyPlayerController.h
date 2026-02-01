// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Player/Loadout/LoadoutColorTypes.h"
#include "LobbyPlayerController.generated.h"

class UCameraComponent;

UCLASS()
class P_MA_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetReady(bool bNewReady);

	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void ShowInviteUI();

	void PreviewEyeColor(const FMaterialParamData& EyeData);
	void PreviewBodyColor(const FMaterialParamData& BodyData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
	void ShowLobbyUI();

protected:
	UPROPERTY(EditAnywhere, Category = "Lobby")
	TSubclassOf<class ULobbyWidgetRoot> LobbyRootWidgetClass;

	UPROPERTY()
	TObjectPtr<ULobbyWidgetRoot> LobbyRootWidgetInstance;

	UPROPERTY(EditAnywhere, Category = "Lobby")
	FName LobbyCameraTag = TEXT("LobbyCamera");

private:
	UFUNCTION()
	void HandleReadyStartClicked();

	UFUNCTION()
	void HandleLoadoutClicked();

	void UpdateLobbyUI();
	void EnterLoadoutView();
	void ExitLoadoutView();
	void UpdateCameraTarget();
	void ApplyPreviewColor(const FMaterialParamDataPair& ColorData);
	void CommitLoadoutColor();

	UFUNCTION(Server, Reliable)
	void ServerSetLoadoutColor(const FMaterialParamDataPair& ColorData);

	FTimerHandle LobbyUiTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	FTransform LoadoutCameraOffset;

	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	float CameraInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	float FovInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	float LobbyFov = 75.0f;

	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	float LoadoutFov = 55.0f;

	UPROPERTY()
	TObjectPtr<AActor> LobbyCameraActor;

	UPROPERTY()
	TObjectPtr<class UCameraComponent> LobbyCameraComponent;

	FTransform LobbyCameraTransform;
	FTransform TargetCameraTransform;
	float TargetFov = 0.0f;
	bool bInLoadoutView = false;

	FMaterialParamDataPair PendingLoadoutColor;
	bool bHasPendingLoadoutColor = false;
};
