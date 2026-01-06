// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformComponent.h"
#include "P_MA/P_MA.h"
#include "NiagaraFunctionLibrary.h"

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
	SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

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

	ReadyWallBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
		
		ReadyWallBox->SetRelativeLocation(FVector(0, 0, BoxWidth * 25));

		// debug
		ReadyWallBox->SetHiddenInGame(false);
	}
}
