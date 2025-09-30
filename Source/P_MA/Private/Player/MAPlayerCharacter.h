// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MACharacter.h"
#include "InputActionValue.h"
#include "GAS/MAGameplayAbilityTypes.h"

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
	UInputAction* MovementInputAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TMap<EMAAbilityInputID, class UInputAction*> GameplayAbilityInputActions;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* GameplayInputMappingContext;
	
	void HandleMoveInput(const FInputActionValue& InputActionValue);
	void HandleAttackInput(const FInputActionValue& InputActionValue);
	void HandleSkillInput(const FInputActionValue& InputActionValue);
	void HandleInteractInput(const FInputActionValue& InputActionValue);
	void HandleMovementInput(const FInputActionValue& InputActionValue);
	void HandleAbilityInput(const FInputActionValue& InputActionValue, EMAAbilityInputID InputID);
	void SetInputEnabledFromPlayerController(bool bEnabled);
	
	/** Cam **/
	bool GetLookDirectionToMouse(FVector& OutDirection) const;
	
	UFUNCTION(Server, Reliable)
	void Server_SetRotation(FVector LookDirection);

	/** Weapon **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UWeaponComponent> WeaponComponent = nullptr;

	/** Stun **/
	virtual void OnStun() override;
	virtual void OnRecoverFromStun() override;
	
	/** Death and Respawn **/
	virtual void OnDead() override;
	virtual void OnRespawn() override;
	virtual void OnGhostMode();

	/** Mini Map 아래 코드는 공부할 필요 없음 강의 에는 없는 코드 입니다 **/
	UPROPERTY(VisibleAnywhere, Category="MinimapCamera")
	class USpringArmComponent* MinimapCameraBoom;

	UPROPERTY(VisibleAnywhere, Category="MinimapCamera")
	class USceneCaptureComponent2D* MinimapCapture;

	UPROPERTY(VisibleAnywhere, Category="MinimapCamera")
	class UPaperSpriteComponent* MinimapSprite;
	/** 여기 위에 까지는 별도의 코드 입니다 **/


	/** 텔레포트 기능 **/
public:
	// 이 함수는 GameplayAbility_Teleport에서 호출됩니다.
	void RequestTeleport(FVector TargetLocation);
protected:
	// 서버가 텔레포트 요청을 받아 모든 클라이언트에게 전파합니다.
	UFUNCTION(Server, Reliable)
	void Server_RequestTeleport(FVector_NetQuantize Location);

	// 서버와 모든 클라이언트에서 실제 텔레포트를 실행합니다.
	UFUNCTION(NetMulticast,Reliable)
	void Multicast_PerformTeleport(FVector_NetQuantize Location);

};
