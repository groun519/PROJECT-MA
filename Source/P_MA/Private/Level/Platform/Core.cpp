// Fill out your copyright notice in the Description page of Project Settings.


#include "Core.h"


ACore::ACore()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACore::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACore::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
