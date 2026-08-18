#include "AI/Monster/MAMonsterCharacterMovementComponent.h"

UMAMonsterCharacterMovementComponent::UMAMonsterCharacterMovementComponent()
{
	// bUseAccelerationForPaths moved into NavMovementProperties in UE 5.8.
	NavMovementProperties.bUseAccelerationForPaths = true;
}
