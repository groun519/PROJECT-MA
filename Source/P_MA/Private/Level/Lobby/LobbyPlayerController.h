#pragma once

#include "CoreMinimal.h"
#include "Player/MAPlayerControllerBase.h"
#include "Player/Loadout/LoadoutTypes.h"
#include "LobbyAvatarState.h"
#include "LobbyPlayerController.generated.h"

class UCameraComponent;
class ULoadoutWidget;
enum class ELoadoutTab : uint8;

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
class P_MA_API ALobbyPlayerController : public AMAPlayerControllerBase
{
	GENERATED_BODY()

public:
	enum class ELoadoutView : uint8
	{
		Head,
		Body,
		Weapon,
		Mount
	};

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetReady(bool bNewReady);

	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);

	void PreviewEyeColor(const FMaterialParamData& EyeData);
	UFUNCTION(BlueprintCallable, Category = "Lobby|Loadout")
	void PreviewEyeShape(FName EyeShapeId);
	void PreviewBodyColor(const FMaterialParamData& BodyData);
	void PreviewWeapon(FName WeaponId, USkeletalMesh* Mesh, const FTransform& Offset);
	void SetPendingMount(FName MountId);
	void ApplyPendingWeaponPreview();
	void ApplyPendingWeaponPreviewDelayed(float DelaySeconds);
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
	void BindLoadoutWidget(ULoadoutWidget& LoadoutWidget);
	void HandleLoadoutTabSelected(ELoadoutTab Tab);
	void EnterLoadoutView();
	void ExitLoadoutView();
	void UpdateCameraTarget();
	void ApplyCameraTransition(const FLoadoutCameraViewSettings& PrevViewSettings, const FLoadoutCameraViewSettings& NextViewSettings);
	void ApplyPreviewColor(const FMaterialParamDataPair& ColorData);
	void ApplyPendingMountPreview();
	void EnsurePendingLoadoutInitialized();
	void CommitPendingLoadout();
	void SetLocalSlotMountPreviewVisible(bool bVisible);
	void SetLocalSlotWeaponPreviewVisible(bool bVisible);

	UFUNCTION(Server, Reliable)
	void ServerSetLoadoutSelection(const FLoadoutSelection& Loadout);

	UFUNCTION(Server, Reliable)
	void ServerSetLobbyState(ELobbyAvatarState NewState);

	UFUNCTION(Client, Reliable)
	void ClientStartLoadingScreen();

	FTimerHandle LobbyUiTimerHandle;
	FTimerHandle WeaponPreviewTimerHandle;

	/** Lobby Loadout Cam Views **/
	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	FLoadoutCameraViewSettings LobbyView;
	
	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	FLoadoutCameraViewSettings LoadoutBodyView;

	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	FLoadoutCameraViewSettings LoadoutHeadView;

	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	FLoadoutCameraViewSettings LoadoutWeaponView;

	UPROPERTY(EditAnywhere, Category = "Lobby|Camera")
	FLoadoutCameraViewSettings LoadoutMountView;

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
	FLoadoutCameraViewSettings ActiveViewSettings;

	FLoadoutSelection PendingLoadout;
	bool bHasPendingLoadout = false;
};
