#include "Level/Lobby/Hub/LobbyHubMagicCircle.h"

#include "P_MA/P_MA.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

ALobbyHubMagicCircle::ALobbyHubMagicCircle()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	CircleMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CircleMeshComponent"));
	CircleMeshComponent->SetupAttachment(RootComponent);
	CircleMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CircleMeshComponent->SetCollisionObjectType(ECC_WorldStatic);
	CircleMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CircleMeshComponent->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Block);

	ReadyAreaComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ReadyAreaComponent"));
	ReadyAreaComponent->SetupAttachment(RootComponent);
	ReadyAreaComponent->InitSphereRadius(250.f);
	ReadyAreaComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ReadyAreaComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	ReadyAreaComponent->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Overlap);
	ReadyAreaComponent->SetGenerateOverlapEvents(true);
	ReadyAreaComponent->OnComponentBeginOverlap.AddDynamic(
		this,
		&ALobbyHubMagicCircle::HandleReadyAreaBeginOverlap);
	ReadyAreaComponent->OnComponentEndOverlap.AddDynamic(
		this,
		&ALobbyHubMagicCircle::HandleReadyAreaEndOverlap);
}

bool ALobbyHubMagicCircle::IsPlayerReady(const APlayerState* PlayerState) const
{
	return PlayerState && ReadyPlayers.Contains(PlayerState);
}

void ALobbyHubMagicCircle::HandleReadyAreaBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	const APawn* Pawn = Cast<APawn>(OtherActor);
	APlayerState* PlayerState = Pawn ? Pawn->GetPlayerState() : nullptr;
	if (PlayerState) AddReadyPlayer(*PlayerState);
}

void ALobbyHubMagicCircle::HandleReadyAreaEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex)
{
	if (!HasAuthority()) return;

	const APawn* Pawn = Cast<APawn>(OtherActor);
	APlayerState* PlayerState = Pawn ? Pawn->GetPlayerState() : nullptr;
	if (PlayerState) RemoveReadyPlayer(*PlayerState);
}

void ALobbyHubMagicCircle::HandleReadyPlayerDestroyed(AActor* DestroyedActor)
{
	if (APlayerState* PlayerState = Cast<APlayerState>(DestroyedActor))
	{
		RemoveReadyPlayer(*PlayerState);
	}
}

void ALobbyHubMagicCircle::AddReadyPlayer(APlayerState& PlayerState)
{
	if (ReadyPlayers.Contains(&PlayerState)) return;

	ReadyPlayers.Add(&PlayerState);
	PlayerState.OnDestroyed.AddDynamic(this, &ALobbyHubMagicCircle::HandleReadyPlayerDestroyed);
}

void ALobbyHubMagicCircle::RemoveReadyPlayer(APlayerState& PlayerState)
{
	if (ReadyPlayers.Remove(&PlayerState) == 0) return;

	PlayerState.OnDestroyed.RemoveDynamic(this, &ALobbyHubMagicCircle::HandleReadyPlayerDestroyed);
}

void ALobbyHubMagicCircle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALobbyHubMagicCircle, ReadyPlayers);
}
