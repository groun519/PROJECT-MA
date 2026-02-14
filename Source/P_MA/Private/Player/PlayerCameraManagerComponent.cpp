// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/PlayerCameraManagerComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/ReadyStateComponent.h"

UPlayerCameraManagerComponent::UPlayerCameraManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UPlayerCameraManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		if (UReadyStateComponent* ReadyComp = Owner->FindComponentByClass<UReadyStateComponent>())
		{
			CachedReadyComponent = ReadyComp;
			ReadyStateChangedHandle = ReadyComp->OnReadyStateChanged.AddUObject(this, &UPlayerCameraManagerComponent::HandleReadyStateChanged);
			HandleReadyStateChanged(ReadyComp->IsReady());
		}
	}
}

void UPlayerCameraManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedReadyComponent.IsValid() && ReadyStateChangedHandle.IsValid())
	{
		CachedReadyComponent->OnReadyStateChanged.Remove(ReadyStateChangedHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UPlayerCameraManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bTransitionActive || !CameraBoom)
	{
		return;
	}

	UpdateTransition(DeltaTime);
}

void UPlayerCameraManagerComponent::Initialize(USpringArmComponent* InCameraBoom, UCameraComponent* InCamera)
{
	CameraBoom = InCameraBoom;
	Camera = InCamera;
}

void UPlayerCameraManagerComponent::CacheReadySettingsFromCurrent()
{
	if (!CameraBoom)
	{
		return;
	}

	ReadySettings.TargetArmLength = CameraBoom->TargetArmLength;
	ReadySettings.BoomPitch = CameraBoom->GetRelativeRotation().Pitch;
	ReadySettings.TargetOffset = CameraBoom->TargetOffset;
	bReadySettingsCached = true;
}

void UPlayerCameraManagerComponent::CacheNotReadySettingsFromCurrent()
{
	if (!CameraBoom)
	{
		return;
	}

	NotReadySettings.TargetArmLength = CameraBoom->TargetArmLength;
	NotReadySettings.BoomPitch = CameraBoom->GetRelativeRotation().Pitch;
	NotReadySettings.TargetOffset = CameraBoom->TargetOffset;
	bNotReadySettingsCached = true;
}

void UPlayerCameraManagerComponent::ApplyReadySettings()
{
	if (!CameraBoom)
	{
		return;
	}

	StartTransition(ReadySettings);
}

void UPlayerCameraManagerComponent::ApplyNotReadySettings()
{
	if (!CameraBoom)
	{
		return;
	}

	StartTransition(NotReadySettings);
}

void UPlayerCameraManagerComponent::HandleReadyStateChanged(bool bIsReady)
{
	if (bIsReady)
	{
		ApplyReadySettings();
		return;
	}

	ApplyNotReadySettings();
}

void UPlayerCameraManagerComponent::ApplySettings(const FCameraBoomOffsetSettings& Settings)
{
	if (!CameraBoom)
	{
		return;
	}

	CameraBoom->TargetArmLength = Settings.TargetArmLength;

	FRotator NewRotation = CameraBoom->GetRelativeRotation();
	NewRotation.Pitch = Settings.BoomPitch;
	CameraBoom->SetRelativeRotation(NewRotation);
	CameraBoom->TargetOffset = Settings.TargetOffset;
}

void UPlayerCameraManagerComponent::StartTransition(const FCameraBoomOffsetSettings& Settings)
{
	if (!CameraBoom)
	{
		return;
	}

	TransitionTargetSettings = Settings;
	TransitionElapsed = 0.f;
	TransitionStartArmLength = CameraBoom->TargetArmLength;
	TransitionStartPitch = CameraBoom->GetRelativeRotation().Pitch;
	TransitionStartTargetOffset = CameraBoom->TargetOffset;
	TransitionStartFOV = Camera ? Camera->FieldOfView : Settings.BaseFOV;
	TransitionBaseFOV = (Settings.BaseFOV > 0.f) ? Settings.BaseFOV : TransitionStartFOV;
	TransitionPulseDelta = Settings.PulseFOVDelta;

	if (Settings.TransitionDuration <= KINDA_SMALL_NUMBER)
	{
		ApplySettings(Settings);
		if (Camera)
		{
			Camera->SetFieldOfView(TransitionBaseFOV);
		}
		bTransitionActive = false;
		SetComponentTickEnabled(false);
		return;
	}

	bTransitionActive = true;
	SetComponentTickEnabled(true);
}

void UPlayerCameraManagerComponent::UpdateTransition(float DeltaTime)
{
	const float Duration = FMath::Max(TransitionTargetSettings.TransitionDuration, KINDA_SMALL_NUMBER);
	TransitionElapsed += DeltaTime;

	const float Alpha = FMath::Clamp(TransitionElapsed / Duration, 0.f, 1.f);
	const float EaseAlpha = GetTransitionEaseAlpha(Alpha);

	ApplyTransitionStep(EaseAlpha, Alpha);

	if (Alpha >= 1.f)
	{
		FinishTransition();
	}
}

void UPlayerCameraManagerComponent::ApplyTransitionStep(float EaseAlpha, float Alpha)
{
	const float NewArmLength = FMath::Lerp(TransitionStartArmLength, TransitionTargetSettings.TargetArmLength, EaseAlpha);
	const float DeltaPitch = FMath::FindDeltaAngleDegrees(TransitionStartPitch, TransitionTargetSettings.BoomPitch);
	const float NewPitch = FMath::UnwindDegrees(TransitionStartPitch + DeltaPitch * EaseAlpha);
	const FVector NewTargetOffset = FMath::Lerp(TransitionStartTargetOffset, TransitionTargetSettings.TargetOffset, EaseAlpha);

	CameraBoom->TargetArmLength = NewArmLength;
	FRotator NewRot = CameraBoom->GetRelativeRotation();
	NewRot.Pitch = NewPitch;
	CameraBoom->SetRelativeRotation(NewRot);
	CameraBoom->TargetOffset = NewTargetOffset;

	if (Camera)
	{
		const float BaseFOV = FMath::Lerp(TransitionStartFOV, TransitionBaseFOV, EaseAlpha);
		const float PulseAlpha = FMath::Sin(Alpha * PI);
		const float NewFOV = BaseFOV + (TransitionPulseDelta * PulseAlpha);
		Camera->SetFieldOfView(NewFOV);
	}
}

void UPlayerCameraManagerComponent::FinishTransition()
{
	bTransitionActive = false;
	SetComponentTickEnabled(false);
	ApplySettings(TransitionTargetSettings);
	if (Camera)
	{
		Camera->SetFieldOfView(TransitionBaseFOV);
	}
}

float UPlayerCameraManagerComponent::GetTransitionEaseAlpha(float Alpha) const
{
	const float EaseExp = FMath::Max(TransitionTargetSettings.TransitionEaseExponent, 0.f);
	if (EaseExp > 0.f)
	{
		return FMath::InterpEaseInOut(0.f, 1.f, Alpha, EaseExp);
	}
	return Alpha;
}
