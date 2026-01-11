// Fill out your copyright notice in the Description page of Project Settings.


#include "ReadyStateComponent.h"
#include "MAPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "P_MA/P_MA.h"

UReadyStateComponent::UReadyStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UReadyStateComponent::ReadyAndMoveIn(FVector InDir, float MovingUnit)
{
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
	Owner->SetActorLocation(FinalLoc, false, nullptr, ETeleportType::TeleportPhysics);
}

void UReadyStateComponent::SetReady(bool bNewReady)
{
	bIsReady = bNewReady;

	AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(GetOwner());
	if (!PlayerCharacter) return;
	
	PlayerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECC_ReadyWall,
		IsReady() ? ECR_Block : ECR_Overlap
		);
}
