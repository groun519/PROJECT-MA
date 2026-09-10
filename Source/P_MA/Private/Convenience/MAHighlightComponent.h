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

	void SetHighlight(UObject& Requester, bool bEnabled,
		const FLinearColor& Color = FLinearColor::White, int32 Priority = 0);

private:
	struct FRequest
	{
		TWeakObjectPtr<UObject> Requester;
		int32 StencilValue = 0;
		int32 Priority = 0;
	};

	static int32 ConvertColorHueToStencilValue(const FLinearColor& Color);
	void ApplyHighlight();

	TArray<TWeakObjectPtr<UPrimitiveComponent>> HighlightTargets;
	TArray<FRequest> HighlightRequests;
};
