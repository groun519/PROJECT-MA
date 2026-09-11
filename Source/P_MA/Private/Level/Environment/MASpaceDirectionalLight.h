#pragma once

#include "CoreMinimal.h"
#include "Engine/DirectionalLight.h"
#include "MASpaceDirectionalLight.generated.h"

/** Owns the authored and runtime Directional Lighting of one Space. */
UCLASS()
class P_MA_API AMASpaceDirectionalLight : public ADirectionalLight
{
	GENERATED_BODY()

public:
	AMASpaceDirectionalLight();

	virtual void PreRegisterAllComponents() override;
	virtual void PostInitializeComponents() override;

	void ActivateLighting();
	void TransitionTo(float Alpha, AMASpaceDirectionalLight* Destination);

private:
	struct FLightingState
	{
		FQuat Rotation = FQuat::Identity;
		FLinearColor EffectiveColor = FLinearColor::White;
		float Intensity = 0.f;
		float IndirectLightingIntensity = 1.f;
		float VolumetricScatteringIntensity = 1.f;
	};

	FLightingState CaptureLightingState() const;
	void ApplyLightingState(const FLightingState& State);
	void SetLightingEnabled(bool bLightingEnabled);

	FLightingState AuthoredLightingState;
};
