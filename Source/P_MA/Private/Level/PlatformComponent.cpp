// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformComponent.h"
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

	// /** Movementing Box **/
	// MovementingBox = CreateDefaultSubobject<UBoxComponent>("MovementingBox");
	// MovementingBox->SetBoxExtent(FVector(10.0f, 10.0f, 10.0f));
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
}

