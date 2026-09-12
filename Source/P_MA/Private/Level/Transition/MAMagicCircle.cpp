#include "Level/Transition/MAMagicCircle.h"

#include "P_MA/P_MA.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Level/Transition/MASpaceTransitionVisibilityComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

AMAMagicCircle::AMAMagicCircle()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	/** Components **/
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	CircleMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CircleMeshComponent"));
	CircleMeshComponent->SetupAttachment(RootComponent);
	CircleMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CircleMeshComponent->SetCollisionObjectType(ECC_WorldStatic);
	CircleMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CircleMeshComponent->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Block);

	/** Player Detection **/
	PlayerAreaComponent = CreateDefaultSubobject<USphereComponent>(TEXT("PlayerAreaComponent"));
	PlayerAreaComponent->SetupAttachment(RootComponent);
	PlayerAreaComponent->InitSphereRadius(250.f);
	PlayerAreaComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PlayerAreaComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	PlayerAreaComponent->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Overlap);
	PlayerAreaComponent->SetGenerateOverlapEvents(true);
	PlayerAreaComponent->OnComponentBeginOverlap.AddDynamic(
		this,
		&AMAMagicCircle::HandlePlayerAreaBeginOverlap);
	PlayerAreaComponent->OnComponentEndOverlap.AddDynamic(
		this,
		&AMAMagicCircle::HandlePlayerAreaEndOverlap);

	/** Transition Visibility **/
	UMASpaceTransitionVisibilityComponent* SpaceTransitionVisibilityComponent =
		CreateDefaultSubobject<UMASpaceTransitionVisibilityComponent>(TEXT("WorldTransitionVisibilityComponent"));
	SpaceTransitionVisibilityComponent->AddTarget(CircleMeshComponent);
}

/** Auto Travel **/

void AMAMagicCircle::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority()) RefreshReadyTimer();
}

void AMAMagicCircle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(ReadyTimer);
	Super::EndPlay(EndPlayReason);
}

void AMAMagicCircle::SetAutoTravelEnabled(const bool bEnabled)
{
	if (!HasAuthority() || bAutoTravelEnabled == bEnabled) return;
	bAutoTravelEnabled = bEnabled;
	RefreshReadyTimer();
}

bool AMAMagicCircle::AreAllPlayersInCircle() const
{
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!GameState || GameState->PlayerArray.IsEmpty()) return false;
	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (!IsPlayerInCircle(PlayerState)) return false;
	}
	return true;
}

void AMAMagicCircle::RefreshReadyTimer()
{
	if (!bAutoTravelEnabled || !AreAllPlayersInCircle())
	{
		GetWorldTimerManager().ClearTimer(ReadyTimer);
		return;
	}
	if (!GetWorldTimerManager().IsTimerActive(ReadyTimer))
	{
		GetWorldTimerManager().SetTimer(ReadyTimer, this, &AMAMagicCircle::HandleAllPlayersReady, 3.f, false);
	}
}

void AMAMagicCircle::HandleAllPlayersReady()
{
	if (!bAutoTravelEnabled || !AreAllPlayersInCircle()) return;
	OnAllPlayersReady.Broadcast();
}

/** Player Detection **/

bool AMAMagicCircle::IsPlayerInCircle(const APlayerState* PlayerState) const
{
	return PlayerState && PlayersInCircle.Contains(PlayerState);
}

void AMAMagicCircle::HandlePlayerAreaBeginOverlap(
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
	if (PlayerState) AddPlayerInCircle(*PlayerState);
}

void AMAMagicCircle::HandlePlayerAreaEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex)
{
	if (!HasAuthority()) return;

	const APawn* Pawn = Cast<APawn>(OtherActor);
	APlayerState* PlayerState = Pawn ? Pawn->GetPlayerState() : nullptr;
	if (PlayerState) RemovePlayerInCircle(*PlayerState);
}

void AMAMagicCircle::HandlePlayerInCircleDestroyed(AActor* DestroyedActor)
{
	if (APlayerState* PlayerState = Cast<APlayerState>(DestroyedActor))
	{
		RemovePlayerInCircle(*PlayerState);
	}
}

void AMAMagicCircle::AddPlayerInCircle(APlayerState& PlayerState)
{
	if (PlayersInCircle.Contains(&PlayerState)) return;

	PlayersInCircle.Add(&PlayerState);
	PlayerState.OnDestroyed.AddDynamic(this, &AMAMagicCircle::HandlePlayerInCircleDestroyed);
	RefreshReadyTimer();
}

void AMAMagicCircle::RemovePlayerInCircle(APlayerState& PlayerState)
{
	if (PlayersInCircle.Remove(&PlayerState) == 0) return;

	PlayerState.OnDestroyed.RemoveDynamic(this, &AMAMagicCircle::HandlePlayerInCircleDestroyed);
	RefreshReadyTimer();
}

/** Transform **/

FTransform AMAMagicCircle::WorldToCircleTransform(const FTransform& WorldTransform) const
{
	FTransform CircleWorldTransform = GetActorTransform();
	CircleWorldTransform.SetScale3D(FVector::OneVector);
	FTransform RelativeTransform = WorldTransform.GetRelativeTransform(CircleWorldTransform);
	RelativeTransform.SetScale3D(FVector::OneVector);
	return RelativeTransform;
}

FTransform AMAMagicCircle::CircleToWorldTransform(const FTransform& CircleTransform) const
{
	FTransform CircleWorldTransform = GetActorTransform();
	CircleWorldTransform.SetScale3D(FVector::OneVector);
	FTransform WorldTransform = CircleTransform * CircleWorldTransform;
	WorldTransform.SetScale3D(FVector::OneVector);
	return WorldTransform;
}

/** Replication **/

void AMAMagicCircle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMAMagicCircle, PlayersInCircle);
}
