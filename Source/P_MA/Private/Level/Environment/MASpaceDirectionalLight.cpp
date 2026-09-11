#include "Level/Environment/MASpaceDirectionalLight.h"

#include "Components/LightComponent.h"
#include "Engine/World.h"

AMASpaceDirectionalLight::AMASpaceDirectionalLight()
{
	SetMobility(EComponentMobility::Movable);
}

void AMASpaceDirectionalLight::PreRegisterAllComponents()
{
	if (UWorld* World = GetWorld(); World && World->IsGameWorld())
	{
		SetLightingEnabled(false);
	}
	Super::PreRegisterAllComponents();
}

void AMASpaceDirectionalLight::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	AuthoredLightingState = CaptureLightingState();
}

void AMASpaceDirectionalLight::ActivateLighting()
{
	SetLightingEnabled(true);
}

void AMASpaceDirectionalLight::TransitionTo(const float Alpha, AMASpaceDirectionalLight* Destination)
{
	const FLightingState& Target = Destination->AuthoredLightingState;

	FLightingState Current;
	Current.Rotation = FQuat::Slerp(AuthoredLightingState.Rotation, Target.Rotation, Alpha);
	Current.EffectiveColor = FMath::Lerp(AuthoredLightingState.EffectiveColor, Target.EffectiveColor, Alpha);
	Current.Intensity = FMath::Lerp(AuthoredLightingState.Intensity, Target.Intensity, Alpha);
	Current.IndirectLightingIntensity = FMath::Lerp(
		AuthoredLightingState.IndirectLightingIntensity,
		Target.IndirectLightingIntensity,
		Alpha);
	Current.VolumetricScatteringIntensity = FMath::Lerp(
		AuthoredLightingState.VolumetricScatteringIntensity,
		Target.VolumetricScatteringIntensity,
		Alpha);
	ApplyLightingState(Current);

	if (Alpha < 1.f) return;
	SetLightingEnabled(false);
	Destination->ActivateLighting();
}

AMASpaceDirectionalLight::FLightingState AMASpaceDirectionalLight::CaptureLightingState() const
{
	const ULightComponent* Light = GetLightComponent();
	FLightingState State;
	State.Rotation = GetActorQuat();
	State.EffectiveColor = Light->GetLightColor();
	if (Light->bUseTemperature) State.EffectiveColor *= Light->GetColorTemperature();
	State.Intensity = Light->Intensity;
	State.IndirectLightingIntensity = Light->IndirectLightingIntensity;
	State.VolumetricScatteringIntensity = Light->VolumetricScatteringIntensity;
	return State;
}

void AMASpaceDirectionalLight::ApplyLightingState(const FLightingState& State)
{
	ULightComponent* Light = GetLightComponent();
	SetActorRotation(State.Rotation);
	Light->SetUseTemperature(false);
	Light->SetLightColor(State.EffectiveColor);
	Light->SetIntensity(State.Intensity);
	Light->SetIndirectLightingIntensity(State.IndirectLightingIntensity);
	Light->SetVolumetricScatteringIntensity(State.VolumetricScatteringIntensity);
}

void AMASpaceDirectionalLight::SetLightingEnabled(const bool bLightingEnabled)
{
	GetLightComponent()->SetVisibility(bLightingEnabled);
}
