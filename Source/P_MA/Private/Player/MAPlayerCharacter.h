// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MACharacter.h"
#include "InputActionValue.h"

#include "MAPlayerCharacter.generated.h"

class UInputAction;

/**
 * 
 */
UCLASS()
class AMAPlayerCharacter : public AMACharacter
{
	GENERATED_BODY()
	
public:
	AMAPlayerCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	class UCameraComponent* Cam;
	
	FVector GetMoveForwardDir() const; 
	FVector GetMoveRightDir() const;
	
	/** Input **/
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveInputAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* AttackInputAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* SkillInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* InteractInputAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* GameplayInputMappingContext;
	
	void HandleMoveInput(const FInputActionValue& InputActionValue);
	void HandleAttackInput(const FInputActionValue& InputActionValue);
	void HandleSkillInput(const FInputActionValue& InputActionValue);
	void HandleInteractInput(const FInputActionValue& InputActionValue);

	/** Cam **/
	bool GetLookDirectionToMouse(FVector& OutDirection) const;
	void UpdateCameraLead(const FVector& LookDirection) const;

	/** Weapon **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UWeaponComponent> WeaponCom = nullptr;

	/** Death and Respawn **/
	virtual void OnDead() override;
	virtual void OnRespawn() override;
};
