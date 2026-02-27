// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/GameOverManagerComponent.h"

UGameOverManagerComponent::UGameOverManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGameOverManagerComponent::TryTriggerGameOver()
{
	// TODO: Implement game over evaluation logic.
}
