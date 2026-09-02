#include "Player/Camera/MAPlayerCameraDirectorComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/Camera/MACameraOcclusionCutoutComponent.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/MAPlayerControllerBase.h"
#include "TimerManager.h"

/** Lifecycle **/
UMAPlayerCameraDirectorComponent::UMAPlayerCameraDirectorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
}

void UMAPlayerCameraDirectorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeactivatePresentationEffects();
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

/** View Target **/
void UMAPlayerCameraDirectorComponent::RefreshPawnCamera()
{
	CameraBoom = nullptr;
	Camera = nullptr;

	AMAPlayerControllerBase* PlayerController = Cast<AMAPlayerControllerBase>(GetOwnerPlayerController());
	if (!PlayerController || !PlayerController->IsLocalController()) return;

	UMACameraOcclusionCutoutComponent* OcclusionCutout = PlayerController->GetCameraOcclusionCutout();
	if (OcclusionCutout) OcclusionCutout->ClearTarget();

	AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(PlayerController->GetPawn());
	if (!PlayerCharacter) return;

	CameraBoom = PlayerCharacter->GetCameraBoom();
	Camera = PlayerCharacter->GetPlayerCamera();
	if (OcclusionCutout) OcclusionCutout->RevealTarget(*PlayerController, *PlayerCharacter);
}

void UMAPlayerCameraDirectorComponent::SwitchToViewTarget(AActor* ViewTarget, float BlendTime)
{
	AMAPlayerControllerBase* PlayerController = Cast<AMAPlayerControllerBase>(GetOwnerPlayerController());
	if (!PlayerController || !PlayerController->IsLocalController() || !ViewTarget) return;

	DeactivatePresentationEffects();
	CancelCameraMovement();
	PlayerController->SetViewTargetWithBlend(ViewTarget, BlendTime);
	if (UMACameraOcclusionCutoutComponent* OcclusionCutout = PlayerController->GetCameraOcclusionCutout())
	{
		OcclusionCutout->RevealTarget(*PlayerController, *ViewTarget);
	}
}

void UMAPlayerCameraDirectorComponent::SwitchToPawnCamera(float BlendTime)
{
	APlayerController* PlayerController = GetOwnerPlayerController();
	if (!PlayerController || !PlayerController->IsLocalController()) return;

	APawn* Pawn = PlayerController->GetPawn();
	if (!Pawn) return;

	DeactivatePresentationEffects();
	CancelCameraMovement();
	RefreshPawnCamera();
	PlayerController->SetViewTargetWithBlend(Pawn, BlendTime);
}

/** Presentation **/
void UMAPlayerCameraDirectorComponent::EnterPresentationView(
	AActor* ViewTarget,
	AActor* Subject,
	const float BlendTime)
{
	APlayerController* PlayerController = GetOwnerPlayerController();
	if (!PlayerController || !PlayerController->IsLocalController() || !ViewTarget || !Subject) return;

	SwitchToViewTarget(ViewTarget, BlendTime);
	ActivatePresentationEffects(*ViewTarget, *Subject);
}

void UMAPlayerCameraDirectorComponent::ExitPresentationView(const float BlendTime)
{
	SwitchToPawnCamera(BlendTime);
}

void UMAPlayerCameraDirectorComponent::ActivatePresentationEffects(AActor& ViewTarget, AActor& Subject)
{
	AMAPlayerControllerBase* PlayerController = Cast<AMAPlayerControllerBase>(GetOwnerPlayerController());
	if (!PlayerController || !PlayerController->IsLocalController()) return;

	if (UMACameraOcclusionCutoutComponent* OcclusionCutout = PlayerController->GetCameraOcclusionCutout())
	{
		OcclusionCutout->RevealTarget(*PlayerController, Subject);
	}

	UCameraComponent* ViewCamera = ViewTarget.FindComponentByClass<UCameraComponent>();
	if (!ViewCamera) return;

	PresentationFillLight = NewObject<USpotLightComponent>(
		&Subject,
		NAME_None,
		RF_Transient);
	if (!PresentationFillLight) return;

	Subject.AddInstanceComponent(PresentationFillLight);
	PresentationFillLight->SetMobility(EComponentMobility::Movable);
	PresentationFillLight->SetCastShadows(false);
	PresentationFillLight->SetIndirectLightingIntensity(0.f);
	PresentationFillLight->SetVolumetricScatteringIntensity(0.f);
	PresentationFillLight->SetIntensity(PresentationSettings.FillLightIntensity);
	PresentationFillLight->SetAttenuationRadius(PresentationSettings.FillLightRadius);
	PresentationFillLight->SetInnerConeAngle(PresentationSettings.FillLightInnerCone);
	PresentationFillLight->SetOuterConeAngle(
		FMath::Max(PresentationSettings.FillLightOuterCone, PresentationSettings.FillLightInnerCone));
	PresentationFillLight->SetLightColor(PresentationSettings.FillLightColor);
	PresentationFillLight->AttachToComponent(
		ViewCamera,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	PresentationFillLight->SetRelativeLocationAndRotation(
		FVector::ZeroVector,
		FRotator::ZeroRotator);
	PresentationFillLight->RegisterComponent();
}

void UMAPlayerCameraDirectorComponent::DeactivatePresentationEffects()
{
	if (IsValid(PresentationFillLight))
	{
		PresentationFillLight->DestroyComponent();
	}
	PresentationFillLight = nullptr;
}

/** Fade **/
void UMAPlayerCameraDirectorComponent::RequestFade(const FMACameraFadeSettings& Settings)
{
	AMAPlayerControllerBase* PlayerController = Cast<AMAPlayerControllerBase>(GetOwnerPlayerController());
	if (!PlayerController) return;

	if (PlayerController->IsLocalController())
	{
		PlayFade(Settings);
		return;
	}

	if (PlayerController->HasAuthority())
	{
		ClientRequestFade(Settings);
	}
}

void UMAPlayerCameraDirectorComponent::ClientRequestFade_Implementation(const FMACameraFadeSettings& Settings)
{
	PlayFade(Settings);
}

void UMAPlayerCameraDirectorComponent::PlayFade(const FMACameraFadeSettings& Settings)
{
	const float FadeInSeconds = Settings.FadeInSeconds;
	FadeOut(
		Settings.FadeOutSeconds,
		[this, FadeInSeconds]()
		{
			FadeIn(FadeInSeconds);
		});
}

void UMAPlayerCameraDirectorComponent::FadeOut(float Duration, TFunction<void()> OnFinished)
{
	StartCameraFade(0.f, 1.f, Duration, true, false, MoveTemp(OnFinished));
}

void UMAPlayerCameraDirectorComponent::FadeIn(float Duration, TFunction<void()> OnFinished)
{
	StartCameraFade(1.f, 0.f, Duration, false, true, MoveTemp(OnFinished));
}

void UMAPlayerCameraDirectorComponent::StartCameraFade(float FromAlpha, float ToAlpha, float Duration, bool bHoldWhenFinished, bool bStopWhenFinished, TFunction<void()> OnFinished)
{
	CancelFadeTransition();
	PendingFadeFinishedAction = MoveTemp(OnFinished);
	bStopCameraFadeOnFinish = bStopWhenFinished;

	APlayerController* PlayerController = GetOwnerPlayerController();
	if (!PlayerController || !PlayerController->PlayerCameraManager)
	{
		HandleFadeFinished();
		return;
	}

	const float SafeDuration = FMath::Max(0.f, Duration);
	if (SafeDuration <= 0.f)
	{
		HandleFadeFinished();
		return;
	}

	PlayerController->PlayerCameraManager->StartCameraFade(FromAlpha, ToAlpha, SafeDuration, FLinearColor::Black, false, bHoldWhenFinished);
	GetWorld()->GetTimerManager().SetTimer(FadeTimerHandle, this, &UMAPlayerCameraDirectorComponent::HandleFadeFinished, SafeDuration, false);
}

void UMAPlayerCameraDirectorComponent::CancelFadeTransition()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FadeTimerHandle);
	}

	if (APlayerController* PlayerController = GetOwnerPlayerController())
	{
		if (PlayerController->PlayerCameraManager)
		{
			PlayerController->PlayerCameraManager->StopCameraFade();
		}
	}

	PendingFadeFinishedAction = nullptr;
	bStopCameraFadeOnFinish = false;
}

void UMAPlayerCameraDirectorComponent::HandleFadeFinished()
{
	TFunction<void()> FinishedAction = MoveTemp(PendingFadeFinishedAction);
	PendingFadeFinishedAction = nullptr;

	const bool bShouldStopFade = bStopCameraFadeOnFinish;
	bStopCameraFadeOnFinish = false;

	if (bShouldStopFade)
	{
		if (APlayerController* PlayerController = GetOwnerPlayerController())
		{
			if (PlayerController->PlayerCameraManager)
			{
				PlayerController->PlayerCameraManager->StopCameraFade();
			}
		}
	}

	if (FinishedAction) FinishedAction();
}

/** Pawn Camera Rig **/
void UMAPlayerCameraDirectorComponent::TransitionPawnCamera(const FMAPlayerCameraRigSettings& Settings)
{
	if (!CameraBoom) RefreshPawnCamera();
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
		if (Camera) Camera->SetFieldOfView(RigTransitionBaseFOV);

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

	if (Alpha >= 1.f) FinishRigTransition();
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

	if (Camera) Camera->SetFieldOfView(RigTransitionBaseFOV);
}

/** External Camera **/
void UMAPlayerCameraDirectorComponent::InterpExternalCameraView(
	const FMACameraViewTarget& Target,
	const FMACameraInterpMoveSettings& Settings)
{
	if (!SetExternalCameraTarget(Target)) return;

	CancelCameraMovement();
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

	CancelCameraMovement();
	SnapExternalCameraToTarget();
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

/** Internal **/
void UMAPlayerCameraDirectorComponent::CancelCameraTransition()
{
	CancelCameraMovement();
	CancelFadeTransition();
}

void UMAPlayerCameraDirectorComponent::CancelCameraMovement()
{
	bRigTransitionActive = false;
	bExternalCameraTransitionActive = false;
	UpdateComponentTickEnabled();
}

void UMAPlayerCameraDirectorComponent::UpdateComponentTickEnabled()
{
	SetComponentTickEnabled(bRigTransitionActive || bExternalCameraTransitionActive);
}

APlayerController* UMAPlayerCameraDirectorComponent::GetOwnerPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}
