#pragma once

#include "CoreMinimal.h"
#include "MACameraTypes.generated.h"

USTRUCT(BlueprintType)
struct FMACameraFadeSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Camera")
	float FadeOutSeconds = 0.2f;

	UPROPERTY(EditAnywhere, Category="Camera")
	float FadeInSeconds = 0.2f;
};

USTRUCT(BlueprintType)
struct FMACameraRigSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Camera")
	float TargetArmLength = 800.f;

	UPROPERTY(EditAnywhere, Category="Camera")
	float BoomPitch = 0.f;

	UPROPERTY(EditAnywhere, Category="Camera")
	FVector TargetOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category="Camera")
	float TransitionDuration = 0.4f;

	UPROPERTY(EditAnywhere, Category="Camera")
	float TransitionEaseExponent = 2.0f;

	UPROPERTY(EditAnywhere, Category="Camera")
	float BaseFOV = 90.f;

	UPROPERTY(EditAnywhere, Category="Camera")
	float PulseFOVDelta = 0.f;
};

USTRUCT(BlueprintType)
struct FMACameraPresentationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Presentation", meta=(ClampMin="0.0"))
	float FillLightIntensity = 2500.f;

	UPROPERTY(EditAnywhere, Category="Presentation", meta=(ClampMin="1.0"))
	float FillLightRadius = 1200.f;

	UPROPERTY(EditAnywhere, Category="Presentation", meta=(ClampMin="0.0", ClampMax="89.0"))
	float FillLightInnerCone = 25.f;

	UPROPERTY(EditAnywhere, Category="Presentation", meta=(ClampMin="1.0", ClampMax="89.0"))
	float FillLightOuterCone = 40.f;

	UPROPERTY(EditAnywhere, Category="Presentation")
	FLinearColor FillLightColor = FLinearColor(1.f, 0.95f, 0.85f);
};
