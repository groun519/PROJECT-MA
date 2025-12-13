// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraSystem.h"
#include "Components/BoxComponent.h"
#include "PlatformComponent.generated.h"


UCLASS(Blueprintable)
class P_MA_API UPlatformComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
	virtual void BeginPlay() override;

public:
	UPlatformComponent();
	
	void EnablePlatform();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UBoxComponent* TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float BoxWidth = 2.f;

	FORCEINLINE FVector GetPlatformBoxExtent(float InHight) { return FVector(BoxWidth, BoxWidth, InHight); }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	TObjectPtr<UNiagaraSystem> EnableEffect;
};
