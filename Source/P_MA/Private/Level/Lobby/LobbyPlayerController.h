// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Player/Loadout/LoadoutColorTypes.h"
#include "LobbyAvatarState.h"
#include "LobbyPlayerController.generated.h"

class UCameraComponent;

USTRUCT(BlueprintType)
struct FCameraFadeSeconds
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	float CameraFadeOutSeconds = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	float CameraFadeInSeconds = 1.0f;
};

USTRUCT(BlueprintType)
struct FLoadoutCameraViewSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	FTransform Offset;

	/** Interp **/
	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	bool bUseInterp = true;

	// if true ?
	UPROPERTY(EditAnywhere, Category = "Lobby|Camera", meta = (EditCondition = "bUseInterp", EditConditionHides))
	float CameraInterpSpeed = 5.0f;

	// if false ?
	UPROPERTY(EditAnywhere, Category = "Lobby|Camera", meta = (EditCondition = "!bUseInterp", EditConditionHides))
	FCameraFadeSeconds FadeSeconds;

	/** Fov **/
	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	float Fov = 55.0f;

	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	float FovInterpSpeed = 5.0f;
};

UCLASS()
class P_MA_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	enum class ELoadoutView : uint8
	{
		Head,
		Body,
		Weapon
	};

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
	void PreviewWeapon(FName WeaponId, USkeletalMesh* Mesh, const FTransform& Offset);
	void SetLoadoutView(ELoadoutView NewView);

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
	void ApplyCameraTransition(const FLoadoutCameraViewSettings& PrevViewSettings, const FLoadoutCameraViewSettings& NextViewSettings);
	void ApplyFadeTransition(const FLoadoutCameraViewSettings& NextViewSettings);
	void ApplyInterpTransition();
	void ApplyInstantCameraTarget();
	void ApplyPreviewColor(const FMaterialParamDataPair& ColorData);
	void CommitLoadoutColor();
	void CommitLoadoutWeapon();
	void TriggerInstantCameraFade(const FLoadoutCameraViewSettings& ViewSettings);

	UFUNCTION(Server, Reliable)
	void ServerSetLoadoutColor(const FMaterialParamDataPair& ColorData);

	UFUNCTION(Server, Reliable)
	void ServerSetLoadoutWeaponId(FName WeaponId);

	UFUNCTION(Server, Reliable)
	void ServerSetLobbyState(ELobbyAvatarState NewState);

	UFUNCTION(Client, Reliable)
	void ClientStartLoadingScreen();

	FTimerHandle LobbyUiTimerHandle;
	FTimerHandle CameraFadeTimerHandle;
	FTimerHandle CameraFadeEndTimerHandle;

	/** Lobby Loadout Cam Views **/
	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	FLoadoutCameraViewSettings LobbyView;
	
	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	FLoadoutCameraViewSettings LoadoutBodyView;

	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	FLoadoutCameraViewSettings LoadoutHeadView;

	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	FLoadoutCameraViewSettings LoadoutWeaponView;

	/** Cam Settings **/
	UPROPERTY()
	TObjectPtr<AActor> LobbyCameraActor;

	UPROPERTY()
	TObjectPtr<class UCameraComponent> LobbyCameraComponent;

	FTransform LobbyCameraTransform;
	FTransform TargetCameraTransform;
	float TargetFov = 0.0f;
	bool bInLoadoutView = false;
	ELoadoutView CurrentLoadoutView = ELoadoutView::Body;
	bool bUseCameraInterp = true;
	FLoadoutCameraViewSettings ActiveViewSettings;
	bool bIsCameraFading = false;

	FMaterialParamDataPair PendingLoadoutColor;
	bool bHasPendingLoadoutColor = false;
	FName PendingWeaponId;
	bool bHasPendingWeapon = false;
};
