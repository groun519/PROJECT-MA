#pragma once

#include "CoreMinimal.h"
#include "Level/Lobby/Hub/LobbyHubInteractableActor.h"
#include "Player/Loadout/LoadoutTypes.h"
#include "LobbyHubLoadoutStation.generated.h"

class AMAPlayerCharacter;
class ALobbyHubPlayerController;
class UCameraComponent;
class ULobbyWidgetRoot;
class ULoadoutWidget;
class USkeletalMesh;

/** World entry point for the Hub's loadout feature. */
UCLASS()
class P_MA_API ALobbyHubLoadoutStation : public ALobbyHubInteractableActor
{
	GENERATED_BODY()

public:
	ALobbyHubLoadoutStation();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleInteract(AMAPlayerCharacter* Interactor);
	void OpenLoadoutFor(AMAPlayerCharacter& Interactor);
	void BindLoadoutWidget(ULoadoutWidget& LoadoutWidget);
	void UpdateLoadoutCamera(const AMAPlayerCharacter& Interactor);
	void EnterLoadoutPresentation(ALobbyHubPlayerController& PlayerController, AMAPlayerCharacter& Interactor);
	void ExitLoadoutPresentation(ALobbyHubPlayerController& PlayerController);
	void SetInteractorRotationLocked(bool bLocked);
	void ApplyPendingLoadout();
	void HandleBodyColorSelected(const FMaterialParamData& BodyData);
	void HandleEyeColorSelected(const FMaterialParamData& EyeData);
	void HandleEyeShapeSelected(FName EyeShapeId);
	void HandleWeaponSelected(FName WeaponId, USkeletalMesh* Mesh, const FTransform& Offset);
	void HandleMountSelected(FName MountId);

	UFUNCTION()
	void CloseLoadout();

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UCameraComponent> LoadoutCameraComponent;

	UPROPERTY(EditInstanceOnly, Category = "Hub|Loadout")
	TSubclassOf<ULobbyWidgetRoot> LoadoutRootWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<ULobbyWidgetRoot> ActiveLoadoutWidget;

	UPROPERTY(EditAnywhere, Category = "Hub|Loadout|Camera", meta = (ClampMin = "0.0"))
	float CameraDistance = 500.f;

	UPROPERTY(EditAnywhere, Category = "Hub|Loadout|Camera")
	float CameraHeight = 180.f;

	UPROPERTY(EditAnywhere, Category = "Hub|Loadout|Camera")
	float CameraTargetHeight = 0.f;

	/** Moves the optical target away from the UI so the character composes on the left. */
	UPROPERTY(EditAnywhere, Category = "Hub|Loadout|Camera")
	float CameraCompositionOffset = 100.f;

	UPROPERTY(EditAnywhere, Category = "Hub|Loadout|Camera", meta = (ClampMin = "0.0"))
	float CameraBlendTime = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Hub|Loadout|Camera", meta = (ClampMin = "5.0", ClampMax = "170.0"))
	float CameraFov = 45.f;

	TWeakObjectPtr<ALobbyHubPlayerController> ActivePlayerController;
	TWeakObjectPtr<AMAPlayerCharacter> ActiveInteractor;
	FLoadoutSelection PendingLoadout;
	bool bRotationLocked = false;
};
