#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Player/Camera/MACameraTypes.h"
#include "MAPlayerCameraDirectorComponent.generated.h"

class UCameraComponent;
class USpringArmComponent;
class AMAPlayerControllerBase;

UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class P_MA_API UMAPlayerCameraDirectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMAPlayerCameraDirectorComponent();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void RefreshPawnCamera();
	void SwitchToViewTarget(AActor* ViewTarget);
	void SwitchToPawnCamera();
	void RequestFade(const FMACameraFadeSettings& Settings);
	void PlayFade(const FMACameraFadeSettings& Settings);
	void FadeOut(float Duration, TFunction<void()> OnFinished = nullptr);
	void FadeIn(float Duration, TFunction<void()> OnFinished = nullptr);
	void TransitionPawnCamera(const FMAPlayerCameraRigSettings& Settings);
	void InterpExternalCameraView(const FMACameraViewTarget& Target, const FMACameraInterpMoveSettings& Settings);
	void TeleportExternalCameraView(const FMACameraViewTarget& Target);

private:
	UFUNCTION(Client, Reliable)
	void ClientRequestFade(const FMACameraFadeSettings& Settings);

	void CancelCameraTransition();
	void CancelCameraMovement();
	void UpdateRigTransition(float DeltaTime);
	void ApplyRigTransitionStep(float EaseAlpha, float Alpha);
	void FinishRigTransition();

	bool SetExternalCameraTarget(const FMACameraViewTarget& Target);
	void SnapExternalCameraToTarget();
	void UpdateExternalCameraTransition(float DeltaTime);
	void StartCameraFade(float FromAlpha, float ToAlpha, float Duration, bool bHoldWhenFinished, bool bStopWhenFinished, TFunction<void()> OnFinished);
	void CancelFadeTransition();
	void HandleFadeFinished();
	void UpdateComponentTickEnabled();
	APlayerController* GetOwnerPlayerController() const;

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

	TWeakObjectPtr<AActor> ExternalCameraActor;
	TWeakObjectPtr<UCameraComponent> ExternalCameraComponent;
	FTransform ExternalCameraTargetTransform;
	float ExternalCameraTargetFov = 0.f;
	FMACameraInterpMoveSettings ExternalCameraInterpSettings;
	bool bExternalCameraTransitionActive = false;

	FTimerHandle FadeTimerHandle;
	TFunction<void()> PendingFadeFinishedAction;
	bool bStopCameraFadeOnFinish = false;
};
