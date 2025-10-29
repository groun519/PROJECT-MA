// Fill out your copyright notice in the Description page of Project Settings.

#include "PlatformRoot.h"

#include "PlatformComponent.h"
#include "PlatformMatrixComponent.h"
#include "Components/ArrowComponent.h"

APlatformRoot::APlatformRoot()
{
	PrimaryActorTick.bCanEverTick = true;

	/** Add Matrix **/
	PlatformMatrixComponent = CreateDefaultSubobject<UPlatformMatrixComponent>("Matrix");
	PlatformMatrixComponent->SetupAttachment(RootComponent);
	
	/** Add Arrow **/
	if (UArrowComponent* Arrow = GetArrowComponent())
	{
		Arrow->ArrowSize = 3.0f;
		Arrow->ArrowColor = FColor::Red;
		Arrow->SetRelativeLocation(FVector(1000.0f, 0.0f, 50.f));
	}
}

void APlatformRoot::BeginPlay()
{
	Super::BeginPlay();
}

void APlatformRoot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlatformRoot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

