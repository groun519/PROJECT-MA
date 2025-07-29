// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MACharacter.h"
#include "InputActionValue.h"

#include "MAPlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class AMAPlayerCharacter : public AMACharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	class UCameraComponent* Cam;

	FVector GetLookRitDir() const;
	FVector GetMoveForwardDir() const;
};
