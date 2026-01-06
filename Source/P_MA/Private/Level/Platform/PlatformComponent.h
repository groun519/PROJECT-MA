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
	void InitPlatform();
	void InitReadyWall();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UBoxComponent* ReadyWallBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float BoxWidth = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	TObjectPtr<UNiagaraSystem> EnableEffect;

private:
	FORCEINLINE FVector GetPlatformBoxExtent(float InHight) { return FVector(BoxWidth, BoxWidth, InHight); }
	FORCEINLINE FVector GetReadyWallBoxExtent() { return FVector(BoxWidth*25, BoxWidth*25, BoxWidth*25*10); }
};
