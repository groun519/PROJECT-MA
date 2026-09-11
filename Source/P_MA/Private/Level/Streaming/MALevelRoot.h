#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MALevelRoot.generated.h"

class AMAMagicCircle;
class AMASpaceDirectionalLight;

/** Represents one streamed Level and exposes its transition circle. */
UCLASS()
class P_MA_API AMALevelRoot : public AActor
{
	GENERATED_BODY()

public:
	AMAMagicCircle* GetTransitionCircle() const { return TransitionCircle; }
	AMASpaceDirectionalLight* GetDirectionalLight() const { return DirectionalLight; }

private:
	UPROPERTY(EditInstanceOnly, Category = "Level")
	TObjectPtr<AMAMagicCircle> TransitionCircle;

	UPROPERTY(EditInstanceOnly, Category = "Level")
	TObjectPtr<AMASpaceDirectionalLight> DirectionalLight;
};
