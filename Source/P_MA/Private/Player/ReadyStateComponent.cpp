// Fill out your copyright notice in the Description page of Project Settings.


#include "ReadyStateComponent.h"
#include "MAPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "P_MA/P_MA.h"
#include "Framework/MAGameMode.h"
#include "Net/UnrealNetwork.h"

UReadyStateComponent::UReadyStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

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
	
	/** Debug **/
	UE_LOG(LogTemp, Warning, TEXT("UReadyStateComponent::ReadyAndMoveIn Execute"));

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

	if (bIsReady == bNewReady)
	{
		return;
	}

	bIsReady = bNewReady;
	HandleReadyStateChanged();
}

void UReadyStateComponent::ServerSetReady_Implementation(bool bNewReady)
{
	SetReady(bNewReady);
}

void UReadyStateComponent::OnRep_IsReady()
{
	HandleReadyStateChanged();
}

void UReadyStateComponent::HandleReadyStateChanged()
{
	AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(GetOwner());
	if (!PlayerCharacter) return;

	PlayerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECC_ReadyWall,
		IsReady() ? ECR_Block : ECR_Overlap
		);

	if (PlayerCharacter->HasAuthority())
	{
		if (AMAGameMode* GameMode = GetWorld()->GetAuthGameMode<AMAGameMode>())
		{
			GameMode->BroadcastReadyCounts();
		}
	}
}

void UReadyStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UReadyStateComponent, bIsReady);
}
