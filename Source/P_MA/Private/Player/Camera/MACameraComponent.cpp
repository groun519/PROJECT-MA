#include "Player/Camera/MACameraComponent.h"

#include "GameFramework/SpringArmComponent.h"

UMACameraComponent::UMACameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UMACameraComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	USpringArmComponent* SpringArm = TransitionSpringArm.Get();
	if (!SpringArm)
	{
		SetComponentTickEnabled(false);
		return;
	}

	const float Duration = FMath::Max(TargetRigSettings.TransitionDuration, KINDA_SMALL_NUMBER);
	TransitionElapsed += DeltaTime;

	const float Alpha = FMath::Clamp(TransitionElapsed / Duration, 0.f, 1.f);
	const float EaseExponent = FMath::Max(TargetRigSettings.TransitionEaseExponent, 0.f);
	const float EaseAlpha = EaseExponent > 0.f
		? FMath::InterpEaseInOut(0.f, 1.f, Alpha, EaseExponent)
		: Alpha;

	ApplyRigStep(EaseAlpha, Alpha);
	if (Alpha >= 1.f)
	{
		FinishRigTransition();
	}
}

void UMACameraComponent::TransitionRig(
	USpringArmComponent& SpringArm,
	const FMACameraRigSettings& Settings)
{
	TransitionSpringArm = &SpringArm;
	TargetRigSettings = Settings;
	TransitionElapsed = 0.f;
	StartArmLength = SpringArm.TargetArmLength;
	StartPitch = SpringArm.GetRelativeRotation().Pitch;
	StartTargetOffset = SpringArm.TargetOffset;
	StartFOV = FieldOfView;
	TargetBaseFOV = Settings.BaseFOV > 0.f ? Settings.BaseFOV : StartFOV;

	if (Settings.TransitionDuration <= KINDA_SMALL_NUMBER)
	{
		FinishRigTransition();
		return;
	}

	SetComponentTickEnabled(true);
}

void UMACameraComponent::ApplyRigStep(const float EaseAlpha, const float Alpha)
{
	USpringArmComponent* SpringArm = TransitionSpringArm.Get();
	if (!SpringArm) return;

	SpringArm->TargetArmLength = FMath::Lerp(StartArmLength, TargetRigSettings.TargetArmLength, EaseAlpha);
	const float PitchDelta = FMath::FindDeltaAngleDegrees(StartPitch, TargetRigSettings.BoomPitch);
	FRotator Rotation = SpringArm->GetRelativeRotation();
	Rotation.Pitch = FMath::UnwindDegrees(StartPitch + PitchDelta * EaseAlpha);
	SpringArm->SetRelativeRotation(Rotation);
	SpringArm->TargetOffset = FMath::Lerp(StartTargetOffset, TargetRigSettings.TargetOffset, EaseAlpha);

	const float BaseFOV = FMath::Lerp(StartFOV, TargetBaseFOV, EaseAlpha);
	SetFieldOfView(BaseFOV + TargetRigSettings.PulseFOVDelta * FMath::Sin(Alpha * PI));
}

void UMACameraComponent::FinishRigTransition()
{
	SetComponentTickEnabled(false);

	if (USpringArmComponent* SpringArm = TransitionSpringArm.Get())
	{
		SpringArm->TargetArmLength = TargetRigSettings.TargetArmLength;
		FRotator Rotation = SpringArm->GetRelativeRotation();
		Rotation.Pitch = TargetRigSettings.BoomPitch;
		SpringArm->SetRelativeRotation(Rotation);
		SpringArm->TargetOffset = TargetRigSettings.TargetOffset;
	}

	SetFieldOfView(TargetBaseFOV);
	TransitionSpringArm.Reset();
}
