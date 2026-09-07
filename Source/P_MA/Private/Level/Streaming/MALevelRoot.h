#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MALevelRoot.generated.h"

class AMAMagicCircle;

/** Represents one streamed Level and exposes its transition circle. */
UCLASS()
class P_MA_API AMALevelRoot : public AActor
{
	GENERATED_BODY()

public:
	AMAMagicCircle* GetTransitionCircle() const { return TransitionCircle; }

private:
	UPROPERTY(EditInstanceOnly, Category = "Level")
	TObjectPtr<AMAMagicCircle> TransitionCircle;
};
