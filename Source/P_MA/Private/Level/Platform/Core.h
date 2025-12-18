// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MACharacter.h"
#include "GameFramework/Character.h"
#include "Player/MAPlayerCharacter.h"
#include "Core.generated.h"

class UInteractComponent;

UCLASS()
class P_MA_API ACore : public AMACharacter
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	ACore();

	UPROPERTY(VisibleAnywhere, Category="MA|Interact") 
	TObjectPtr<UInteractComponent> InteractComp;

private:
	void HandleInteract(AMAPlayerCharacter* Interactor);
};
