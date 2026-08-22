#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MANPC.generated.h"

class UMAHighlightComponent;
class UMAInteractableComponent;
class USkeletalMeshComponent;

UCLASS(Abstract)
class P_MA_API AMANPC : public AActor
{
	GENERATED_BODY()

public:
	AMANPC();

protected:
	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UMAInteractableComponent> InteractableComponent;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UMAHighlightComponent> HighlightComponent;
};
