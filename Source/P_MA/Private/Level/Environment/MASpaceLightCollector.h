#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MASpaceLightCollector.generated.h"

class ULevel;
class ULocalLightComponent;

/** Collects one Level's local lights and applies a scale to their original intensity. */
UCLASS()
class P_MA_API UMASpaceLightCollector : public UObject
{
	GENERATED_BODY()

public:
	void Collect(ULevel& Level, float IntensityScale);
	void SetIntensityScale(float IntensityScale);
	// Releases the work list without changing lights in a Level being unloaded.
	void Reset();

private:
	struct FLightState
	{
		// Construction reruns replace BP components at the same object path.
		TSoftObjectPtr<ULocalLightComponent> Light;
		float OriginalIntensity;
	};

	TArray<FLightState> Lights;
};
