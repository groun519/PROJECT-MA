// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformComponent.h"
#include "P_MA/P_MA.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/ReadyStateComponent.h"

class AMAPlayerCharacter;

UPlatformComponent::UPlatformComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	/** Static Mesh **/
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
	TEXT("/Engine/BasicShapes/Cube.Cube")
	);
	if (CubeMesh.Succeeded())
	{
		UStaticMeshComponent::SetStaticMesh(CubeMesh.Object);
	}
	SetRelativeScale3D(GetPlatformBoxExtent(0.5f));

	/** Trigger Box **/
	// ReadyWallBox = CreateDefaultSubobject<UBoxComponent>("ReadyWallBox");
	// ReadyWallBox->SetupAttachment(this);
	// ReadyWallBox->SetBoxExtent(GetPlatformBoxExtent(BoxWidth));
	// ReadyWallBox->SetCollisionObjectType(ECC_ReadyWall);
	// ReadyWallBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// ReadyWallBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	// ReadyWallBox->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Block);
	//TriggerBox->SetRelativeLocation()
}

void UPlatformComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UPlatformComponent::EnablePlatform()
{
	SetVisibility(true, true);
	bIsEnablePlatform = true;

	// Enable ReadyWall to Ready
	ReadyWallBox->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Overlap);
	ReadyWallBox->OnComponentBeginOverlap.AddDynamic(this, &UPlatformComponent::OnWallOverlap);

	// Effect
	if (EnableEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			EnableEffect,
			this,                  
			NAME_None,           
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepWorldPosition,
			true    
		);
	}
}

void UPlatformComponent::InitPlatform()
{
	
}

void UPlatformComponent::InitReadyWall()
{
	if (ReadyWallBox) return;

	ReadyWallBox = NewObject<UBoxComponent>(this, TEXT("ReadyWallBox"));
	if (ReadyWallBox)
	{
		ReadyWallBox->bEditableWhenInherited = true;

		ReadyWallBox->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		ReadyWallBox->RegisterComponent();
		
		ReadyWallBox->SetBoxExtent(GetReadyWallBoxExtent());
		ReadyWallBox->SetCollisionObjectType(ECC_ReadyWall);
		ReadyWallBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ReadyWallBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		ReadyWallBox->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Block);
		
		ReadyWallBox->SetRelativeLocation(FVector(0, 0, BoxWidth * 25 * 10));

		// debug
		ReadyWallBox->SetHiddenInGame(false);
	}
}

void UPlatformComponent::OnWallOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMAPlayerCharacter* Player = Cast<AMAPlayerCharacter>(OtherActor);
	if (!Player || Player->GetReadyComponent()->IsReady()) return;

	// 예외처리 디버깅 하자
	
	Player->GetReadyComponent()->SetReady(true);

	FVector ToCenter = (GetComponentLocation() - Player->GetActorLocation());
	ToCenter.Z = 0; 
	FVector LaunchVel = ToCenter.GetSafeNormal() * 300.f;
	LaunchVel.Z = 300.f; 
	Player->LaunchCharacter(LaunchVel, true, true);
	
	// FVector TargetLocation = GetComponentLocation();
	// FVector CurrentLocation = Player->GetActorLocation();
	// TargetLocation.Z = CurrentLocation.Z;
	// Player->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::None);
	// if (auto* Movement = Player->GetCharacterMovement())
	// {
	// 	Movement->Velocity = FVector::ZeroVector;
	// }
}