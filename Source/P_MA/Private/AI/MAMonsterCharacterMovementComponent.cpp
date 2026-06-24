#include "AI/MAMonsterCharacterMovementComponent.h"

UMAMonsterCharacterMovementComponent::UMAMonsterCharacterMovementComponent()
{
	// bUseAccelerationForPaths is protected, so monster-only path acceleration is set through this subclass.
	bUseAccelerationForPaths = true;
}
