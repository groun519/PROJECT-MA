// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlatformRoot.generated.h"

class ACore;
class UPlatformMatrixComponent;

UCLASS()
class P_MA_API APlatformRoot : public AActor
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
public:
	APlatformRoot();

	/** Matrix **/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UPlatformMatrixComponent* PlatformMatrixComponent;

	/** Core **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ACore> CoreClass;

	/** Atts Set **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed = 1000.f;
	
private:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;
	
	int32 CurSector = 0;
	float Distance = 0.f;

	/** Height System **/
	float CurHeight = 0.f;
	float MovingHeight = 150.f;
	float WaitingHeight = 0.f;

	/** Core **/
	void SpawnCore();
};
