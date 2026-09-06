#include "ReadyStateComponent.h"

#include "Player/MAPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "P_MA/P_MA.h"
#include "Framework/MAGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Player/Camera/MACameraComponent.h"
#include "TimerManager.h"

UReadyStateComponent::UReadyStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UReadyStateComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(this, &UReadyStateComponent::ApplyReadyCameraTransition);
	}
}

void UReadyStateComponent::ReadyAndMoveIn(FVector InDir, float MovingUnit)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		ServerReadyAndMoveIn(InDir, MovingUnit);
		return;
	}

	if (IsReady()) return;
	SetReady(true);

	AActor* Owner = GetOwner();
	if (!Owner) return;
	
	/** Multiple MovingUnit by PlayerLoc **/
	FVector HorizontalTarget = Owner->GetActorLocation() + (InDir * MovingUnit);

	/** Raycast Init **/
	FHitResult HitResult;
	FVector TraceStart = HorizontalTarget + FVector(0.f, 0.f, 500.f); 
	FVector TraceEnd = HorizontalTarget - FVector(0.f, 0.f, 500.f);
    
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	/** Cast and Hit **/
	FVector FinalLoc = Owner->GetActorLocation(); // <-이동해야 할 위치 저장
	
	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params))
	{
		if (ACharacter* Character = Cast<ACharacter>(Owner))
		{
			float HalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			FinalLoc = HitResult.Location + FVector(0.f, 0.f, HalfHeight);
		}
		else
		{
			FinalLoc = HitResult.Location;
		}
	}

	/** Teleport **/
	ACharacter* Character = Cast<ACharacter>(Owner);
	if (!Character) return;

	Character->GetCharacterMovement()->StopMovementImmediately();
	Character->TeleportTo(FinalLoc, Character->GetActorRotation(), false, true);
	Character->ForceNetUpdate();

	if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
	{
		PC->ClientSetLocation(FinalLoc, Character->GetActorRotation());
	}
}

void UReadyStateComponent::ServerReadyAndMoveIn_Implementation(FVector InDir, float MovingUnit)
{
	ReadyAndMoveIn(InDir, MovingUnit);
}

void UReadyStateComponent::SetReady(bool bNewReady)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		ServerSetReady(bNewReady);
		return;
	}

	if (bIsReady == bNewReady) return;

	bIsReady = bNewReady;
	HandleReadyStateChanged();
}

void UReadyStateComponent::ServerSetReady_Implementation(bool bNewReady)
{
	SetReady(bNewReady);
}

void UReadyStateComponent::SetLoopReady(bool bNewReady)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		ServerSetLoopReady(bNewReady);
		return;
	}

	if (bIsLoopReady == bNewReady) return;

	bIsLoopReady = bNewReady;
	HandleLoopReadyStateChanged();
}

void UReadyStateComponent::ServerSetLoopReady_Implementation(bool bNewReady)
{
	SetLoopReady(bNewReady);
}

void UReadyStateComponent::OnRep_IsReady()
{
	HandleReadyStateChanged();
}

void UReadyStateComponent::OnRep_IsLoopReady()
{
	HandleLoopReadyStateChanged();
}

void UReadyStateComponent::HandleReadyStateChanged()
{
	AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(GetOwner());
	if (!PlayerCharacter) return;
	const ECollisionResponse NewResponse = IsReady() ? ECR_Block : ECR_Overlap;

	PlayerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECC_ReadyWall,
		NewResponse
		);

	if (PlayerCharacter->HasAuthority())
	{
		if (AMAGameMode* GameMode = GetWorld()->GetAuthGameMode<AMAGameMode>())
		{
			GameMode->BroadcastReadyCounts();
		}
	}

	ApplyReadyCameraTransition();
	OnReadyStateChanged.Broadcast(IsReady());
}

void UReadyStateComponent::HandleLoopReadyStateChanged()
{
	OnLoopReadyStateChanged.Broadcast(IsLoopReady());
}

void UReadyStateComponent::ApplyReadyCameraTransition()
{
	const AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(GetOwner());
	if (!PlayerCharacter || !PlayerCharacter->IsLocallyControlled()) return;

	UMACameraComponent* Camera = PlayerCharacter->GetPlayerCamera();
	USpringArmComponent* CameraBoom = PlayerCharacter->GetCameraBoom();
	if (!Camera || !CameraBoom) return;

	Camera->TransitionRig(*CameraBoom, IsReady() ? ReadyCameraSettings : NotReadyCameraSettings);
}

void UReadyStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UReadyStateComponent, bIsReady);
	DOREPLIFETIME(UReadyStateComponent, bIsLoopReady);
}
