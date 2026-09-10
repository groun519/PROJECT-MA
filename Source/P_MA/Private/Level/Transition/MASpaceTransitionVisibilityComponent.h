#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MASpaceTransitionVisibilityComponent.generated.h"

class UPrimitiveComponent;

/** Keeps explicitly selected meshes visible while the Space transition mask is closed. */
UCLASS()
class P_MA_API UMASpaceTransitionVisibilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void AddTarget(UPrimitiveComponent* Target);
	void SetVisibleThroughTransition(bool bVisible);

private:
	TArray<TWeakObjectPtr<UPrimitiveComponent>> Targets;
};
