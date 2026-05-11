#include "Player/Camera/MAPlayerCameraDirectorComponent.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/MAPlayerCharacter.h"
#include "TimerManager.h"

UMAPlayerCameraDirectorComponent::UMAPlayerCameraDirectorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UMAPlayerCameraDirectorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelCameraTransition();
	Super::EndPlay(EndPlayReason);
}

void UMAPlayerCameraDirectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bRigTransitionActive && CameraBoom)
	{
		UpdateRigTransition(DeltaTime);
	}

	if (bExternalCameraTransitionActive)
	{
		UpdateExternalCameraTransition(DeltaTime);
	}
}

void UMAPlayerCameraDirectorComponent::RefreshPawnCamera()
{
	CameraBoom = nullptr;
	Camera = nullptr;

	APlayerController* PlayerController = GetOwnerPlayerController();
	if (!PlayerController || !PlayerController->IsLocalController()) return;

	AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(PlayerController->GetPawn());
	if (!PlayerCharacter) return;

	CameraBoom = PlayerCharacter->GetCameraBoom();
	Camera = PlayerCharacter->GetPlayerCamera();
}

void UMAPlayerCameraDirectorComponent::InterpExternalCameraView(const FMACameraViewTarget& Target, const FMACameraInterpMoveSettings& Settings)
{
	if (!SetExternalCameraTarget(Target)) return;

	CancelCameraTransition();

	ExternalCameraInterpSettings = Settings;

	if (Settings.CameraInterpSpeed <= 0.f && Settings.FovInterpSpeed <= 0.f)
	{
		SnapExternalCameraToTarget();
		return;
	}

	bExternalCameraTransitionActive = true;
	UpdateComponentTickEnabled();
}

void UMAPlayerCameraDirectorComponent::TeleportExternalCameraView(const FMACameraViewTarget& Target)
{
	if (!SetExternalCameraTarget(Target)) return;

	CancelCameraTransition();
	SnapExternalCameraToTarget();
}

void UMAPlayerCameraDirectorComponent::TeleportExternalCameraView(const FMACameraViewTarget& Target, const FMACameraTeleportSettings& Settings)
{
	if (!SetExternalCameraTarget(Target)) return;

	CancelCameraTransition();
	if (Settings.bUseFade)
	{
		StartTeleportFade(Settings.FadeSettings);
		return;
	}

	SnapExternalCameraToTarget();
}

void UMAPlayerCameraDirectorComponent::CancelCameraTransition()
{
	bRigTransitionActive = false;
	bExternalCameraTransitionActive = false;
	CancelFadeTransition();
	UpdateComponentTickEnabled();
}

void UMAPlayerCameraDirectorComponent::StartTeleportFade(const FMACameraFadeSettings& Settings)
{
	APlayerController* PlayerController = GetOwnerPlayerController();
	if (!PlayerController || !PlayerController->PlayerCameraManager)
	{
		SnapExternalCameraToTarget();
		return;
	}

	const float SafeFadeOutSeconds = FMath::Max(0.f, Settings.FadeOutSeconds);
	PendingFadeInSeconds = FMath::Max(0.f, Settings.FadeInSeconds);

	if (SafeFadeOutSeconds <= 0.f && PendingFadeInSeconds <= 0.f)
	{
		HandleFadeOutFinished();
		return;
	}

	if (SafeFadeOutSeconds > 0.f)
	{
		PlayerController->PlayerCameraManager->StartCameraFade(0.f, 1.f, SafeFadeOutSeconds, FLinearColor::Black, false, true);
		GetWorld()->GetTimerManager().SetTimer(FadeOutTimerHandle, this, &UMAPlayerCameraDirectorComponent::HandleFadeOutFinished, SafeFadeOutSeconds, false);
		return;
	}

	HandleFadeOutFinished();
}

void UMAPlayerCameraDirectorComponent::CancelFadeTransition()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FadeOutTimerHandle);
		World->GetTimerManager().ClearTimer(FadeEndTimerHandle);
	}

	if (APlayerController* PlayerController = GetOwnerPlayerController())
	{
		if (PlayerController->PlayerCameraManager)
		{
			PlayerController->PlayerCameraManager->StopCameraFade();
		}
	}

	PendingFadeInSeconds = 0.f;
}

void UMAPlayerCameraDirectorComponent::TransitionPawnCamera(const FMAPlayerCameraRigSettings& Settings)
{
	if (!CameraBoom)
	{
		RefreshPawnCamera();
	}

	if (!CameraBoom) return;

	RigTransitionTargetSettings = Settings;
	RigTransitionElapsed = 0.f;
	RigTransitionStartArmLength = CameraBoom->TargetArmLength;
	RigTransitionStartPitch = CameraBoom->GetRelativeRotation().Pitch;
	RigTransitionStartTargetOffset = CameraBoom->TargetOffset;
	RigTransitionStartFOV = Camera ? Camera->FieldOfView : Settings.BaseFOV;
	RigTransitionBaseFOV = (Settings.BaseFOV > 0.f) ? Settings.BaseFOV : RigTransitionStartFOV;

	if (Settings.TransitionDuration <= KINDA_SMALL_NUMBER)
	{
		CameraBoom->TargetArmLength = Settings.TargetArmLength;
		FRotator NewRotation = CameraBoom->GetRelativeRotation();
		NewRotation.Pitch = Settings.BoomPitch;
		CameraBoom->SetRelativeRotation(NewRotation);
		CameraBoom->TargetOffset = Settings.TargetOffset;
		if (Camera)
		{
			Camera->SetFieldOfView(RigTransitionBaseFOV);
		}
		bRigTransitionActive = false;
		UpdateComponentTickEnabled();
		return;
	}

	bRigTransitionActive = true;
	UpdateComponentTickEnabled();
}

void UMAPlayerCameraDirectorComponent::UpdateRigTransition(float DeltaTime)
{
	const float Duration = FMath::Max(RigTransitionTargetSettings.TransitionDuration, KINDA_SMALL_NUMBER);
	RigTransitionElapsed += DeltaTime;

	const float Alpha = FMath::Clamp(RigTransitionElapsed / Duration, 0.f, 1.f);
	const float EaseExp = FMath::Max(RigTransitionTargetSettings.TransitionEaseExponent, 0.f);
	const float EaseAlpha = EaseExp > 0.f ? FMath::InterpEaseInOut(0.f, 1.f, Alpha, EaseExp) : Alpha;

	ApplyRigTransitionStep(EaseAlpha, Alpha);

	if (Alpha >= 1.f)
	{
		FinishRigTransition();
	}
}

void UMAPlayerCameraDirectorComponent::ApplyRigTransitionStep(float EaseAlpha, float Alpha)
{
	const float NewArmLength = FMath::Lerp(RigTransitionStartArmLength, RigTransitionTargetSettings.TargetArmLength, EaseAlpha);
	const float DeltaPitch = FMath::FindDeltaAngleDegrees(RigTransitionStartPitch, RigTransitionTargetSettings.BoomPitch);
	const float NewPitch = FMath::UnwindDegrees(RigTransitionStartPitch + DeltaPitch * EaseAlpha);
	const FVector NewTargetOffset = FMath::Lerp(RigTransitionStartTargetOffset, RigTransitionTargetSettings.TargetOffset, EaseAlpha);

	CameraBoom->TargetArmLength = NewArmLength;
	FRotator NewRot = CameraBoom->GetRelativeRotation();
	NewRot.Pitch = NewPitch;
	CameraBoom->SetRelativeRotation(NewRot);
	CameraBoom->TargetOffset = NewTargetOffset;

	if (!Camera) return;

	const float BaseFOV = FMath::Lerp(RigTransitionStartFOV, RigTransitionBaseFOV, EaseAlpha);
	const float PulseAlpha = FMath::Sin(Alpha * PI);
	Camera->SetFieldOfView(BaseFOV + (RigTransitionTargetSettings.PulseFOVDelta * PulseAlpha));
}

void UMAPlayerCameraDirectorComponent::FinishRigTransition()
{
	bRigTransitionActive = false;
	UpdateComponentTickEnabled();
	CameraBoom->TargetArmLength = RigTransitionTargetSettings.TargetArmLength;

	FRotator NewRotation = CameraBoom->GetRelativeRotation();
	NewRotation.Pitch = RigTransitionTargetSettings.BoomPitch;
	CameraBoom->SetRelativeRotation(NewRotation);
	CameraBoom->TargetOffset = RigTransitionTargetSettings.TargetOffset;

	if (Camera)
	{
		Camera->SetFieldOfView(RigTransitionBaseFOV);
	}
}

bool UMAPlayerCameraDirectorComponent::SetExternalCameraTarget(const FMACameraViewTarget& Target)
{
	if (!Target.CameraActor) return false;

	ExternalCameraActor = Target.CameraActor;
	ExternalCameraComponent = Target.CameraComponent;
	ExternalCameraTargetTransform = Target.Transform;
	ExternalCameraTargetFov = Target.Fov;
	return true;
}

void UMAPlayerCameraDirectorComponent::SnapExternalCameraToTarget()
{
	AActor* CameraActor = ExternalCameraActor.Get();
	if (!CameraActor) return;

	CameraActor->SetActorLocationAndRotation(
		ExternalCameraTargetTransform.GetLocation(),
		ExternalCameraTargetTransform.Rotator()
	);

	UCameraComponent* CameraComponent = ExternalCameraComponent.Get();
	if (CameraComponent && ExternalCameraTargetFov > 0.f)
	{
		CameraComponent->SetFieldOfView(ExternalCameraTargetFov);
	}
}

void UMAPlayerCameraDirectorComponent::UpdateExternalCameraTransition(float DeltaTime)
{
	AActor* CameraActor = ExternalCameraActor.Get();
	if (!CameraActor)
	{
		bExternalCameraTransitionActive = false;
		UpdateComponentTickEnabled();
		return;
	}

	const float CameraInterpSpeed = ExternalCameraInterpSettings.CameraInterpSpeed;
	if (CameraInterpSpeed > 0.f)
	{
		const FVector NewLocation = FMath::VInterpTo(
			CameraActor->GetActorLocation(),
			ExternalCameraTargetTransform.GetLocation(),
			DeltaTime,
			CameraInterpSpeed
		);
		const FRotator NewRotation = FMath::RInterpTo(
			CameraActor->GetActorRotation(),
			ExternalCameraTargetTransform.Rotator(),
			DeltaTime,
			CameraInterpSpeed
		);
		CameraActor->SetActorLocationAndRotation(NewLocation, NewRotation);
	}
	else
	{
		CameraActor->SetActorLocationAndRotation(
			ExternalCameraTargetTransform.GetLocation(),
			ExternalCameraTargetTransform.Rotator()
		);
	}

	UCameraComponent* CameraComponent = ExternalCameraComponent.Get();
	const float FovInterpSpeed = ExternalCameraInterpSettings.FovInterpSpeed;
	if (CameraComponent && ExternalCameraTargetFov > 0.f)
	{
		const float NewFov = (FovInterpSpeed > 0.f)
			? FMath::FInterpTo(CameraComponent->FieldOfView, ExternalCameraTargetFov, DeltaTime, FovInterpSpeed)
			: ExternalCameraTargetFov;
		CameraComponent->SetFieldOfView(NewFov);
	}

	const bool bLocationReached = CameraActor->GetActorLocation().Equals(ExternalCameraTargetTransform.GetLocation(), 0.1f);
	const bool bRotationReached = CameraActor->GetActorRotation().Equals(ExternalCameraTargetTransform.Rotator(), 0.1f);
	const bool bFovReached = !CameraComponent || ExternalCameraTargetFov <= 0.f || FMath::IsNearlyEqual(CameraComponent->FieldOfView, ExternalCameraTargetFov, 0.1f);
	if (bLocationReached && bRotationReached && bFovReached)
	{
		SnapExternalCameraToTarget();
		bExternalCameraTransitionActive = false;
		UpdateComponentTickEnabled();
	}
}

void UMAPlayerCameraDirectorComponent::HandleFadeOutFinished()
{
	SnapExternalCameraToTarget();

	APlayerController* PlayerController = GetOwnerPlayerController();
	if (PlayerController && PlayerController->PlayerCameraManager && PendingFadeInSeconds > 0.f)
	{
		PlayerController->PlayerCameraManager->StartCameraFade(1.f, 0.f, PendingFadeInSeconds, FLinearColor::Black, false, false);
		GetWorld()->GetTimerManager().SetTimer(FadeEndTimerHandle, this, &UMAPlayerCameraDirectorComponent::HandleFadeFinished, PendingFadeInSeconds, false);
		return;
	}

	HandleFadeFinished();
}

void UMAPlayerCameraDirectorComponent::HandleFadeFinished()
{
	if (APlayerController* PlayerController = GetOwnerPlayerController())
	{
		if (PlayerController->PlayerCameraManager)
		{
			PlayerController->PlayerCameraManager->StopCameraFade();
		}
	}

	PendingFadeInSeconds = 0.f;
}

void UMAPlayerCameraDirectorComponent::UpdateComponentTickEnabled()
{
	SetComponentTickEnabled(bRigTransitionActive || bExternalCameraTransitionActive);
}

APlayerController* UMAPlayerCameraDirectorComponent::GetOwnerPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}
