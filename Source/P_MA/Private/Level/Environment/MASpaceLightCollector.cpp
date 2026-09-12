#include "Level/Environment/MASpaceLightCollector.h"

#include "Components/LocalLightComponent.h"
#include "Engine/Level.h"
#include "GameFramework/Actor.h"

void UMASpaceLightCollector::Collect(ULevel& Level, const float IntensityScale)
{
	Lights.Reset();
	for (AActor* Actor : Level.Actors)
	{
		if (!IsValid(Actor)) continue;
		TInlineComponentArray<ULocalLightComponent*> ActorLights(Actor);
		for (ULocalLightComponent* Light : ActorLights)
		{
			if (Light->Mobility == EComponentMobility::Static) continue;
			Lights.Add({Light, Light->Intensity});
		}
	}
	SetIntensityScale(IntensityScale);
}

void UMASpaceLightCollector::SetIntensityScale(const float IntensityScale)
{
	for (const FLightState& State : Lights)
	{
		if (ULocalLightComponent* Light = State.Light.Get())
		{
			Light->SetIntensity(State.OriginalIntensity * IntensityScale);
		}
	}
}

void UMASpaceLightCollector::Reset()
{
	Lights.Reset();
}
