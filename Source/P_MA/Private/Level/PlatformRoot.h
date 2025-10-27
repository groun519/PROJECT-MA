// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlatformRoot.generated.h"

class UPlatformMatrixComponent;

UCLASS()
class P_MA_API APlatformRoot : public ACharacter
{
	GENERATED_BODY()

public:
	APlatformRoot();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Matrix **/
	UPROPERTY()
	UPlatformMatrixComponent* PlatformMatrixComponent;
};
