#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MAHighlightComponent.generated.h"

class UPrimitiveComponent;

UCLASS()
class P_MA_API UMAHighlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void AddTarget(UPrimitiveComponent* Target);
	void RemoveTarget(UPrimitiveComponent* Target);
	void SetHighlighted(bool bHighlighted, int32 StencilValue = 251);
	void SetTargetHighlighted(UPrimitiveComponent* Target, bool bHighlighted, int32 StencilValue = 251);

private:
	TArray<TWeakObjectPtr<UPrimitiveComponent>> HighlightTargets;
};
