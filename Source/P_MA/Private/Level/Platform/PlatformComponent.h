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
	/** Init and ects **/
	UPlatformComponent();
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
	float BoxWidth = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	TObjectPtr<UNiagaraSystem> EnableEffect;

	/** Platform **/
	void EnablePlatform();
	FORCEINLINE bool IsEnablePlatform() const { return bIsEnablePlatform; }
	
	/** Ready Wall **/
	void InitReadyWall();
	
	UFUNCTION()
	void OnWallOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
		);

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
	UBoxComponent* ReadyWallBox;

	FORCEINLINE bool CanMoveIn() const { return bCanMoveIn; }
	FORCEINLINE void SetCanMoveIn(bool bNew) { bCanMoveIn = bNew; }
	
private:
	/** Platform **/
	bool bIsEnablePlatform = false;
	FORCEINLINE FVector GetPlatformBoxExtent(float InHight) { return FVector(BoxWidth, BoxWidth, InHight); }

	/** Ready Wall **/
	FORCEINLINE FVector GetReadyWallBoxExtent() { return FVector(BoxWidth*25, BoxWidth*25, BoxWidth*25*10); }

	bool bCanMoveIn = false;
};
