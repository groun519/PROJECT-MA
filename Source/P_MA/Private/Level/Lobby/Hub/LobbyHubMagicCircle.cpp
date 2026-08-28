#include "Level/Lobby/Hub/LobbyHubMagicCircle.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

ALobbyHubMagicCircle::ALobbyHubMagicCircle()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	CircleMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CircleMeshComponent"));
	CircleMeshComponent->SetupAttachment(RootComponent);
	CircleMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
