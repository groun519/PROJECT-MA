// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformComponent.h"

#include "IAutomationControllerManager.h"
#include "NiagaraFunctionLibrary.h"


void UPlatformComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

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
	TriggerBox = CreateDefaultSubobject<UBoxComponent>("TriggerBox");
	TriggerBox->SetupAttachment(this);
	TriggerBox->SetBoxExtent(GetPlatformBoxExtent(BoxWidth));
	//TriggerBox->SetRelativeLocation()
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

