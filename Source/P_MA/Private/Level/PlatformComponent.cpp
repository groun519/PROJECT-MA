// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformComponent.h"
#include "NiagaraFunctionLibrary.h"


UPlatformComponent::UPlatformComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
	TEXT("/Engine/BasicShapes/Cube.Cube")
	);
	if (CubeMesh.Succeeded())
	{
		UStaticMeshComponent::SetStaticMesh(CubeMesh.Object);
	}
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

