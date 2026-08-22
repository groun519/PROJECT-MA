#include "NPC/MANPC.h"

#include "Components/SkeletalMeshComponent.h"
#include "Convenience/MAHighlightComponent.h"
#include "Convenience/MAInteractableComponent.h"

AMANPC::AMANPC()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	InteractableComponent = CreateDefaultSubobject<UMAInteractableComponent>(TEXT("InteractableComponent"));
	RootComponent = InteractableComponent;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HighlightComponent = CreateDefaultSubobject<UMAHighlightComponent>(TEXT("HighlightComponent"));
	HighlightComponent->AddTarget(MeshComponent);
	InteractableComponent->CALL_SETUP_HIGHLIGHTER(HighlightComponent);
}
