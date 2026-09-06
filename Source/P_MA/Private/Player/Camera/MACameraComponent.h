#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Player/Camera/MACameraTypes.h"
#include "MACameraComponent.generated.h"

class USpringArmComponent;

/** Camera-owned interpolation for the rig attached to this camera. */
UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class P_MA_API UMACameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	UMACameraComponent();

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	void TransitionRig(USpringArmComponent& SpringArm, const FMACameraRigSettings& Settings);

private:
	void ApplyRigStep(float EaseAlpha, float Alpha);
	void FinishRigTransition();

	TWeakObjectPtr<USpringArmComponent> TransitionSpringArm;
	FMACameraRigSettings TargetRigSettings;
	float TransitionElapsed = 0.f;
	float StartArmLength = 0.f;
	float StartPitch = 0.f;
	FVector StartTargetOffset = FVector::ZeroVector;
	float StartFOV = 90.f;
	float TargetBaseFOV = 90.f;
};
