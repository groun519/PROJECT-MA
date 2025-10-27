// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MACharacter.h"
#include "InputActionValue.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MAPlayerCharacter.generated.h"

class UInputAction;
class UNiagaraComponent;
// 모든 충전/홀딩 스킬 UI가 공유할 델리게이트를 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMAChargeAbilityStateChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMAChargeAbilityUpdate, float, ChargePercentage);

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

	UFUNCTION(Exec)
	void SetSkillBehavior(const FString& SkillClassName, const FString& BehaviorTagString);
	UFUNCTION(Server, Reliable)
	void Server_SetSkillBehavior(const FString& SkillClassName, const FString& BehaviorTagString);
	
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	class UCameraComponent* Cam;
	
	FVector GetMoveForwardDir() const; 
	FVector GetMoveRightDir() const;

	UPROPERTY()
	class UMAPlayerAttributeSet* PlayerAttributeSet;
	
	/** Input **/
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* InteractInputAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TMap<EMAAbilityInputID, class UInputAction*> GameplayAbilityInputActions;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* GameplayInputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	float RotationInterpSpeed = 15.f;
	
	void HandleMoveInput(const FInputActionValue& InputActionValue);
	void HandleInteractInput(const FInputActionValue& InputActionValue);
	void HandleAbilityInput(const FInputActionValue& InputActionValue, EMAAbilityInputID InputID);
public:
	void SetInputEnabledFromPlayerController(bool bEnabled);
	/** Cam **/
	bool GetLookDirectionToMouse(FVector& OutDirection) const;
private:
	
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

	/*************************************************************/
	/**								SKILL						**/
	/*************************************************************/
public:
	UPROPERTY(EditDefaultsOnly, Category = "State")
	FGameplayTag RotationLockTag;

	UPROPERTY(EditDefaultsOnly, Category = "State")
	FGameplayTag RushingTag;

	// Charge스킬을 위한 코드
	UPROPERTY(BlueprintAssignable, Category = "Abilities | UI")
	FOnMAChargeAbilityStateChanged OnChargeAbilityStarted;

	UPROPERTY(BlueprintAssignable, Category = "Abilities | UI")
	FOnMAChargeAbilityUpdate OnChargeAbilityUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Abilities | UI")
	FOnMAChargeAbilityStateChanged OnChargeAbilityEnded;
	// 여기까지
	
};
