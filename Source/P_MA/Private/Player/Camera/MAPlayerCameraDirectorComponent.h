#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Player/Camera/MACameraTypes.h"
#include "MAPlayerCameraDirectorComponent.generated.h"

class UCameraComponent;
class UMACameraOcclusionCutoutComponent;
class USpotLightComponent;
class USpringArmComponent;
class AMAPlayerControllerBase;

UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class P_MA_API UMAPlayerCameraDirectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Lifecycle **/
	UMAPlayerCameraDirectorComponent();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** View Target **/
	void RefreshPawnCamera();
	void SwitchToViewTarget(AActor* ViewTarget, float BlendTime = 0.f);
	void SwitchToPawnCamera(float BlendTime = 0.f);

	/** Presentation **/
	void EnterPresentationView(AActor* ViewTarget, AActor* Subject, float BlendTime = 0.f);
	void ExitPresentationView(float BlendTime = 0.f);

	/** Fade **/
	void RequestFade(const FMACameraFadeSettings& Settings);
	void PlayFade(const FMACameraFadeSettings& Settings);
	void FadeOut(float Duration, TFunction<void()> OnFinished = nullptr);
	void FadeIn(float Duration, TFunction<void()> OnFinished = nullptr);

	/** Pawn Camera Rig **/
	void TransitionPawnCamera(const FMAPlayerCameraRigSettings& Settings);

	/** External Camera **/
	void InterpExternalCameraView(const FMACameraViewTarget& Target, const FMACameraInterpMoveSettings& Settings);
	void TeleportExternalCameraView(const FMACameraViewTarget& Target);

private:
	/** Fade **/
	UFUNCTION(Client, Reliable)
	void ClientRequestFade(const FMACameraFadeSettings& Settings);
	void StartCameraFade(float FromAlpha, float ToAlpha, float Duration, bool bHoldWhenFinished, bool bStopWhenFinished, TFunction<void()> OnFinished);
	void CancelFadeTransition();
	void HandleFadeFinished();

	/** Pawn Camera Rig **/
	void UpdateRigTransition(float DeltaTime);
	void ApplyRigTransitionStep(float EaseAlpha, float Alpha);
	void FinishRigTransition();

	/** External Camera **/
	bool SetExternalCameraTarget(const FMACameraViewTarget& Target);
	void SnapExternalCameraToTarget();
	void UpdateExternalCameraTransition(float DeltaTime);

	/** Presentation **/
	void ActivatePresentationEffects(AActor& ViewTarget, AActor& Subject);
	void DeactivatePresentationEffects();
	void EnsurePresentationCutoutComponent();

	/** Internal **/
	void CancelCameraTransition();
	void CancelCameraMovement();
	void UpdateComponentTickEnabled();
	APlayerController* GetOwnerPlayerController() const;

	/** Pawn Camera Rig **/
	UPROPERTY()
	TObjectPtr<USpringArmComponent> CameraBoom = nullptr;

	UPROPERTY()
	TObjectPtr<UCameraComponent> Camera = nullptr;

	bool bRigTransitionActive = false;
	float RigTransitionElapsed = 0.f;
	float RigTransitionStartArmLength = 0.f;
	float RigTransitionStartPitch = 0.f;
	FVector RigTransitionStartTargetOffset = FVector::ZeroVector;
	float RigTransitionStartFOV = 90.f;
	float RigTransitionBaseFOV = 90.f;
	FMAPlayerCameraRigSettings RigTransitionTargetSettings;

	/** External Camera **/
	TWeakObjectPtr<AActor> ExternalCameraActor;
	TWeakObjectPtr<UCameraComponent> ExternalCameraComponent;
	FTransform ExternalCameraTargetTransform;
	float ExternalCameraTargetFov = 0.f;
	FMACameraInterpMoveSettings ExternalCameraInterpSettings;
	bool bExternalCameraTransitionActive = false;

	/** Presentation **/
	UPROPERTY(Transient)
	TObjectPtr<UMACameraOcclusionCutoutComponent> PresentationCutoutComponent = nullptr;

	/** Owned by the visible subject while active because Controller-owned scene components never render. */
	UPROPERTY(Transient)
	TObjectPtr<USpotLightComponent> PresentationFillLight = nullptr;

	UPROPERTY(EditAnywhere, Category = "Camera|Presentation")
	FMACameraPresentationSettings PresentationSettings;

	/** Fade **/
	FTimerHandle FadeTimerHandle;
	TFunction<void()> PendingFadeFinishedAction;
	bool bStopCameraFadeOnFinish = false;
};
