#include "Level/Lobby/Hub/LobbyHubInteractableActor.h"

#include "Components/StaticMeshComponent.h"
#include "Convenience/MAHighlightComponent.h"
#include "Convenience/MAInteractableComponent.h"

ALobbyHubInteractableActor::ALobbyHubInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	InteractableComponent = CreateDefaultSubobject<UMAInteractableComponent>(TEXT("InteractableComponent"));
	RootComponent = InteractableComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HighlightComponent = CreateDefaultSubobject<UMAHighlightComponent>(TEXT("HighlightComponent"));
	HighlightComponent->AddTarget(MeshComponent);
	InteractableComponent->CALL_SETUP_HIGHLIGHTER(HighlightComponent);
}
