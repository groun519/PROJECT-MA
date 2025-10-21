// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraSystem.h"
#include "PlatformComponent.generated.h"


UCLASS(Blueprintable)
class P_MA_API UPlatformComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UPlatformComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void EnablePlatform();

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> SpawnEffect;
};
