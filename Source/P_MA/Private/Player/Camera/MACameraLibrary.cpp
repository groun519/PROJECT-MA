#include "Player/Camera/MACameraLibrary.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/PlayerController.h"
#include "Player/Camera/MACameraTypes.h"

void FMACameraLibrary::SwitchViewTarget(
	APlayerController& PlayerController,
	AActor& ViewTarget,
	const float BlendTime)
{
	if (!PlayerController.IsLocalController()) return;

	const float SafeBlendTime = FMath::Max(0.f, BlendTime);
	if (SafeBlendTime <= 0.f || PlayerController.GetViewTarget() == &ViewTarget)
	{
		PlayerController.SetViewTarget(&ViewTarget);
		return;
	}

	if (!PlayerController.PlayerCameraManager) return;
	PlayerController.SetViewTargetWithBlend(
		&ViewTarget,
		SafeBlendTime,
		EViewTargetBlendFunction::VTBlend_EaseInOut,
		2.f);
}

void FMACameraLibrary::SwitchToPawn(APlayerController& PlayerController, const float BlendTime)
{
	if (APawn* Pawn = PlayerController.GetPawn())
	{
		SwitchViewTarget(PlayerController, *Pawn, BlendTime);
	}
}

void FMACameraLibrary::FadeOut(APlayerController& PlayerController, const float Duration)
{
	if (!PlayerController.IsLocalController() || !PlayerController.PlayerCameraManager || Duration <= 0.f) return;
	PlayerController.PlayerCameraManager->StartCameraFade(
		0.f,
		1.f,
		Duration,
		FLinearColor::Black,
		false,
		true);
}

void FMACameraLibrary::FadeIn(APlayerController& PlayerController, const float Duration)
{
	if (!PlayerController.IsLocalController() || !PlayerController.PlayerCameraManager || Duration <= 0.f) return;
	PlayerController.PlayerCameraManager->StartCameraFade(
		1.f,
		0.f,
		Duration,
		FLinearColor::Black,
		false,
		false);
}

void FMACameraLibrary::StopFade(APlayerController& PlayerController)
{
	if (PlayerController.IsLocalController() && PlayerController.PlayerCameraManager)
	{
		PlayerController.PlayerCameraManager->StopCameraFade();
	}
}

USpotLightComponent* FMACameraLibrary::CreatePresentationFillLight(
	AActor& Owner,
	USceneComponent& AttachParent,
	const FMACameraPresentationSettings& Settings)
{
	USpotLightComponent* FillLight = NewObject<USpotLightComponent>(&Owner, NAME_None, RF_Transient);
	if (!FillLight) return nullptr;

	Owner.AddInstanceComponent(FillLight);
	FillLight->SetMobility(EComponentMobility::Movable);
	FillLight->SetCastShadows(false);
	FillLight->SetIndirectLightingIntensity(0.f);
	FillLight->SetVolumetricScatteringIntensity(0.f);
	FillLight->SetIntensity(Settings.FillLightIntensity);
	FillLight->SetAttenuationRadius(Settings.FillLightRadius);
	FillLight->SetInnerConeAngle(Settings.FillLightInnerCone);
	FillLight->SetOuterConeAngle(FMath::Max(Settings.FillLightOuterCone, Settings.FillLightInnerCone));
	FillLight->SetLightColor(Settings.FillLightColor);
	FillLight->AttachToComponent(&AttachParent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	FillLight->RegisterComponent();
	return FillLight;
}

void FMACameraLibrary::DestroyPresentationFillLight(USpotLightComponent* FillLight)
{
	if (IsValid(FillLight))
	{
		FillLight->DestroyComponent();
	}
}
