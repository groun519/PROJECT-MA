#include "Convenience/MAHighlightComponent.h"

#include "Components/PrimitiveComponent.h"

void UMAHighlightComponent::AddTarget(UPrimitiveComponent* Target)
{
	if (!ensureMsgf(Target, TEXT("Highlight target is null."))) return;

	HighlightTargets.AddUnique(Target);
}

void UMAHighlightComponent::RemoveTarget(UPrimitiveComponent* Target)
{
	if (!ensureMsgf(Target, TEXT("Highlight target is null."))) return;

	HighlightTargets.Remove(Target);
	Target->SetRenderCustomDepth(false);
}

void UMAHighlightComponent::SetHighlighted(bool bHighlighted, int32 StencilValue)
{
	for (const TWeakObjectPtr<UPrimitiveComponent>& Target : HighlightTargets)
	{
		if (UPrimitiveComponent* Primitive = Target.Get())
		{
			SetTargetHighlighted(Primitive, bHighlighted, StencilValue);
		}
	}
}

void UMAHighlightComponent::SetTargetHighlighted(UPrimitiveComponent* Target, bool bHighlighted, int32 StencilValue)
{
	if (!ensureMsgf(Target, TEXT("Highlight target is null."))) return;

	Target->SetRenderCustomDepth(bHighlighted);
	if (bHighlighted) Target->SetCustomDepthStencilValue(StencilValue);
}
