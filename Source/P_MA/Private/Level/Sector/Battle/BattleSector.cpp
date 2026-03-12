// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleSector.h"


ABattleSector::ABattleSector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABattleSector::BeginPlay()
{
	Super::BeginPlay();
}

void ABattleSector::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}
