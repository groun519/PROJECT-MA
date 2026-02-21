// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCameraManagerComponent.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UReadyStateComponent;

USTRUCT(BlueprintType)
struct FCameraBoomOffsetSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Camera|Ready")
	float TargetArmLength = 800.f;

	UPROPERTY(EditAnywhere, Category="Camera|Ready")
	float BoomPitch = 0.f;

	UPROPERTY(EditAnywhere, Category="Camera|Ready")
	FVector TargetOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category="Camera|Ready")
	float TransitionDuration = 0.4f;

	UPROPERTY(EditAnywhere, Category="Camera|Ready")
	float TransitionEaseExponent = 2.0f;

	UPROPERTY(EditAnywhere, Category="Camera|Ready")
	float BaseFOV = 90.f;

	UPROPERTY(EditAnywhere, Category="Camera|Ready")
	float PulseFOVDelta = 0.f;
};

UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class P_MA_API UPlayerCameraManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerCameraManagerComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Camera")
	void Initialize(USpringArmComponent* InCameraBoom, UCameraComponent* InCamera);

	UFUNCTION(BlueprintCallable, Category="Camera|Ready")
	void CacheReadySettingsFromCurrent();

	UFUNCTION(BlueprintCallable, Category="Camera|Ready")
	void CacheNotReadySettingsFromCurrent();

	UFUNCTION(BlueprintCallable, Category="Camera|Ready")
	void ApplyReadySettings();

	UFUNCTION(BlueprintCallable, Category="Camera|Ready")
	void ApplyNotReadySettings();

private:
	void ApplySettings(const FCameraBoomOffsetSettings& Settings);
	void StartTransition(const FCameraBoomOffsetSettings& Settings);
	void HandleReadyStateChanged(bool bIsReady);

	/** Tick Logic **/
	void UpdateTransition(float DeltaTime);
	void ApplyTransitionStep(float EaseAlpha, float Alpha);
	void FinishTransition();
	float GetTransitionEaseAlpha(float Alpha) const;
	/**/

	UPROPERTY()
	USpringArmComponent* CameraBoom = nullptr;

	UPROPERTY()
	UCameraComponent* Camera = nullptr;

	UPROPERTY(EditAnywhere, Category="Camera|Ready")
	FCameraBoomOffsetSettings ReadySettings;

	UPROPERTY(EditAnywhere, Category="Camera|Ready")
	FCameraBoomOffsetSettings NotReadySettings;

	UPROPERTY(VisibleAnywhere, Category="Camera|Ready")
	bool bReadySettingsCached = false;

	UPROPERTY(VisibleAnywhere, Category="Camera|Ready")
	bool bNotReadySettingsCached = false;

	FDelegateHandle ReadyStateChangedHandle;

	TWeakObjectPtr<UReadyStateComponent> CachedReadyComponent;

	bool bTransitionActive = false;
	float TransitionElapsed = 0.f;
	float TransitionStartArmLength = 0.f;
	float TransitionStartPitch = 0.f;
	FVector TransitionStartTargetOffset = FVector::ZeroVector;
	float TransitionStartFOV = 90.f;
	float TransitionBaseFOV = 90.f;
	float TransitionPulseDelta = 0.f;
	FCameraBoomOffsetSettings TransitionTargetSettings;
};
