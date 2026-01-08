// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MACharacter.h"
#include "InputActionValue.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "Inventory/SkillBookComponent.h"
#include "MAPlayerCharacter.generated.h"

class UInputAction;
class UNiagaraComponent;
class UInteractComponent;
class UReadyStateComponent;

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
	void SetBehavior(const FString& SkillClassName, const FString& BehaviorTagString);
	UFUNCTION(Server, Reliable)
	void Server_SetBehavior(const FString& SkillClassName, const FString& BehaviorTagString);
	UFUNCTION(Exec)
	void SetAttribute(const FString& SkillClassName, const FString& AttributeName);
	UFUNCTION(Server, Reliable)
	void Server_SetAttribute(const FString& SkillClassName, const FString& AttributeName);
	UFUNCTION(Exec)
	void SetUtility(const FString& SkillClassName, const FString& UtilityName);
	UFUNCTION(Server, Reliable)
	void Server_SetUtility(const FString& SkillClassName, const FString& UtilityName);

	/** Ready State Component **/
	FORCEINLINE UReadyStateComponent* GetReadyComponent(){ return ReadyStateComponent; }
	
private:
	/** Ready State Component **/
	UPROPERTY(VisibleDefaultsOnly, Category = "Ready")
	UReadyStateComponent* ReadyStateComponent;

	/** Cam **/
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
	UInputAction* UseInventoryItemAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TMap<EMAAbilityInputID, class UInputAction*> GameplayAbilityInputActions;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* GameplayInputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	float RotationInterpSpeed = 15.f;
	
	void HandleMoveInput(const FInputActionValue& InputActionValue);
	void HandleInteractInput(const FInputActionValue& InputActionValue);
	void HandleAbilityInput(const FInputActionValue& InputActionValue, EMAAbilityInputID InputID);
	void UseInventoryItem(const FInputActionValue& InputActionValue);
public:
	void SetInputEnabledFromPlayerController(bool bEnabled);
	void SnapRotationToMouse();

	/** Interact **/
	UFUNCTION()
	void SetCurrentInteractComp(UInteractComponent* NewComp);

	UFUNCTION()
	void ClearCurrentInteractComp(UInteractComponent* Comp);

	UPROPERTY(Transient)
	TWeakObjectPtr<UInteractComponent> CurrentInteractComp;
	
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

	/** MiniMap **/
	UPROPERTY(VisibleAnywhere, Category="MinimapCamera")
	class USpringArmComponent* MinimapCameraBoom;

	UPROPERTY(VisibleAnywhere, Category="MinimapCamera")
	class USceneCaptureComponent2D* MinimapCapture;

	// UPROPERTY(VisibleAnywhere, Category="MinimapCamera")
	// class UPaperSpriteComponent* MinimapSprite;
	// ㄴ 미사용, 주석처리.
	/****/

	/*************************************************************/
	/*                      Inventory                            */
	/*************************************************************/
	
private:
	class UInventoryComponent* InventoryComponent;

	UPROPERTY(VisibleAnywhere, Category = "Skill")
	class USkillBookComponent* SkillBookComponent;

	/*************************************************************/
	/**								SKILL						**/
	/*************************************************************/
public:
	UPROPERTY(EditDefaultsOnly, Category = "State")
	FGameplayTag RotationLockTag;

	UPROPERTY(EditDefaultsOnly, Category = "State")
	FGameplayTag RushingTag;

	USkillBookComponent* GetSkillBookComponent() const { return SkillBookComponent; }

	// Charge스킬을 위한 코드
	UPROPERTY(BlueprintAssignable, Category = "Abilities | UI")
	FOnMAChargeAbilityStateChanged OnChargeAbilityStarted;

	UPROPERTY(BlueprintAssignable, Category = "Abilities | UI")
	FOnMAChargeAbilityUpdate OnChargeAbilityUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Abilities | UI")
	FOnMAChargeAbilityStateChanged OnChargeAbilityEnded;
	// 여기까지
};
