#pragma once

#include "CoreMinimal.h"

class AActor;
class APlayerController;
class USceneComponent;
class USpotLightComponent;
struct FMACameraPresentationSettings;

/** Stateless camera operations shared by independent feature owners. */
class P_MA_API FMACameraLibrary final
{
public:
	static void SwitchViewTarget(APlayerController& PlayerController, AActor& ViewTarget, float BlendTime = 0.f);
	static void SwitchToPawn(APlayerController& PlayerController, float BlendTime = 0.f);
	static void FadeOut(APlayerController& PlayerController, float Duration);
	static void FadeIn(APlayerController& PlayerController, float Duration);
	static void StopFade(APlayerController& PlayerController);

	static USpotLightComponent* CreatePresentationFillLight(
		AActor& Owner,
		USceneComponent& AttachParent,
		const FMACameraPresentationSettings& Settings);
	static void DestroyPresentationFillLight(USpotLightComponent* FillLight);
};
