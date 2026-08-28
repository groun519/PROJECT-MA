#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyHubInteractableActor.generated.h"

class UMAHighlightComponent;
class UMAInteractableComponent;
class UStaticMeshComponent;

/** Common physical presentation for interactable feature entries in the Lobby Hub. */
UCLASS(Abstract)
class P_MA_API ALobbyHubInteractableActor : public AActor
{
	GENERATED_BODY()

public:
	ALobbyHubInteractableActor();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UMAInteractableComponent> InteractableComponent;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UMAHighlightComponent> HighlightComponent;
};
