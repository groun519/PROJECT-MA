#include "Level/Transition/MASpaceTransitionVisibilityComponent.h"

#include "Components/PrimitiveComponent.h"
#include "MARenderStencil.h"

void UMASpaceTransitionVisibilityComponent::AddTarget(UPrimitiveComponent* Target)
{
	if (!ensure(Target)) return;
	Targets.AddUnique(Target);
}

void UMASpaceTransitionVisibilityComponent::SetVisibleThroughTransition(const bool bVisible)
{
	for (const TWeakObjectPtr<UPrimitiveComponent>& Target : Targets)
	{
		if (UPrimitiveComponent* Primitive = Target.Get())
		{
			FMARenderStencil::SetTransitionVisible(*Primitive, bVisible);
		}
	}
}
