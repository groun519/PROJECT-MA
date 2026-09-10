#include "Level/Transition/MAMagicCircle.h"

#include "P_MA/P_MA.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Level/Transition/MASpaceTransitionVisibilityComponent.h"
#include "Net/UnrealNetwork.h"

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
}

void AMAMagicCircle::RemovePlayerInCircle(APlayerState& PlayerState)
{
	if (PlayersInCircle.Remove(&PlayerState) == 0) return;

	PlayerState.OnDestroyed.RemoveDynamic(this, &AMAMagicCircle::HandlePlayerInCircleDestroyed);
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
