// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MACharacter.h"
#include "InputActionValue.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "Inventory/SkillBookComponent.h"
#include "Loadout/Data/LoadoutWeaponData.h"
#include "MAPlayerCharacter.generated.h"

class UInputAction;
class UNiagaraComponent;
class UAnimMontage;
class UInteractComponent;
class UReadyStateComponent;
class UReadyRideComponent;
class UReadyCheckWidgetComponent;
class USkeletalMeshComponent;
class AMAPlayerState;
class UPlayerCameraManagerComponent;

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
	AMAPlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void Tick(float DeltaTime) override;
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void OnRep_PlayerState() override;
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void BaseChange() override;

	void SetInputEnabledFromPlayerController(bool bEnabled);
	void SnapRotationToMouse();

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

	UPROPERTY(EditDefaultsOnly, Category = "State")
	FGameplayTag RotationLockTag;

	UPROPERTY(EditDefaultsOnly, Category = "State")
	FGameplayTag RushingTag;

	/** Ready State Component **/
	FORCEINLINE UReadyStateComponent* GetReadyStateComponent() const { return ReadyStateComponent; }

	/** Ready Ride Component **/
	FORCEINLINE UReadyRideComponent* GetReadyRideComponent() const { return ReadyRideComponent; }
	FORCEINLINE USkeletalMeshComponent* GetMountMesh() const { return MountMesh; }
	FORCEINLINE float GetRideHorizontalInput() const { return RideHorizontalInput; }

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
	/** Ready State Component **/
	UPROPERTY(VisibleDefaultsOnly, Category = "Ready")
	UReadyStateComponent* ReadyStateComponent;

	/** Ready Ride Component **/
	UPROPERTY(VisibleDefaultsOnly, Category = "Ready")
	UReadyRideComponent* ReadyRideComponent;

	/** Ready Check Widget **/
	UPROPERTY(VisibleDefaultsOnly, Category = "UI")
	UReadyCheckWidgetComponent* ReadyCheckWidget;

	/** Mount **/
	UPROPERTY(VisibleDefaultsOnly, Category = "Mount")
	USkeletalMeshComponent* MountMesh;

	/** Cam **/
	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	class UCameraComponent* Cam;

	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	UPlayerCameraManagerComponent* PlayerCameraManagerComponent;
	
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

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (ClampMin = "0.0"))
	float RotationNetSendInterval = 0.05f;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (ClampMin = "0.0"))
	float RotationNetSendYawThreshold = 1.5f;
	
	void HandleMoveInput(const FInputActionValue& InputActionValue);
	void HandleInteractInput(const FInputActionValue& InputActionValue);
	void HandleAbilityInput(const FInputActionValue& InputActionValue, EMAAbilityInputID InputID);
	void UseInventoryItem(const FInputActionValue& InputActionValue);
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Player Rotate **/
	void UpdateRotationByReadyRide(float DeltaTime);
	void TrySendRotationToServer(const FVector& LookDirection);

	UFUNCTION(Server, Unreliable)
	void Server_SetRotation(FVector LookDirection);

	float LastRotationNetSendTime = -1000.f;
	float LastSentRotationYaw = 0.f;
	bool bHasSentRotationYaw = false;
	float RideHorizontalInput = 0.f;

	/** Loadout **/
	void BindLoadoutDelegates();
	void ApplyLoadoutFromPlayerState();
	void HandleLoadoutChanged(const FLoadoutSelection& Loadout);
	void HandleLoadoutColorChanged(const FMaterialParamDataPair& ColorData);
	void HandleLoadoutEyeShapeChanged(FName EyeShapeId);
	void HandleLoadoutWeaponChanged(FName WeaponId);
	void HandleLoadoutMountChanged(FName MountId);

	UPROPERTY(Transient)
	FGameplayAbilitySpecHandle CurrentBasicAttackHandle;
	void EquipWeaponFromData(const struct FLoadoutWeaponDataRow* WeaponData);
	
	FDelegateHandle LoadoutChangedHandle;

	UPROPERTY()
	TObjectPtr<AMAPlayerState> CachedLoadoutPlayerState;

	/** Weapon **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UWeaponComponent> WeaponComponent = nullptr;

	UPROPERTY(Transient)
	FGameplayAbilitySpecHandle CurrentAttackAbilityHandle;

	/** Stun **/
	virtual void OnStun() override;
	virtual void OnRecoverFromStun() override;
	
	/** Death and Respawn **/
	virtual void OnDead() override;
	virtual void OnRespawn() override;
	void EnableInputAfterRespawnMontage();

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeadColorSaturationScale = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TObjectPtr<class UNiagaraSystem> RespawnVFX = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TObjectPtr<UAnimMontage> RespawnMontage = nullptr;

	FTimerHandle RespawnInputEnableTimerHandle;

	/** MiniMap **/
	UPROPERTY(VisibleAnywhere, Category="MinimapCamera")
	class USpringArmComponent* MinimapCameraBoom;

	UPROPERTY(VisibleAnywhere, Category="MinimapCamera")
	class USceneCaptureComponent2D* MinimapCapture;

	UPROPERTY(EditDefaultsOnly, Category="MinimapCamera", meta=(ClampMin="0.01"))
	float MinimapCaptureInterval = 0.05f;
	float MinimapCaptureAccumulatedTime = 0.f;

	UPROPERTY(VisibleAnywhere, Category="MinimapCamera")
	class UPaperSpriteComponent* MinimapSprite;

	void InitializeMinimapCapture();
	void TickMinimapCapture(float DeltaTime);

	/** Inventory **/
	class UInventoryComponent* InventoryComponent;

	/** SkillBook **/
	UPROPERTY(VisibleAnywhere, Category = "Skill")
	class USkillBookComponent* SkillBookComponent;
public:
	USkillBookComponent* GetSkillBookComponent() const { return SkillBookComponent; }
	class UWeaponComponent* GetWeaponComponent() const { return WeaponComponent; }
	
	/** Skill **/
	// Charge스킬을 위한 코드
	UPROPERTY(BlueprintAssignable, Category = "Abilities | UI")
	FOnMAChargeAbilityStateChanged OnChargeAbilityStarted;

	UPROPERTY(BlueprintAssignable, Category = "Abilities | UI")
	FOnMAChargeAbilityUpdate OnChargeAbilityUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Abilities | UI")
	FOnMAChargeAbilityStateChanged OnChargeAbilityEnded;
	// 여기까지

	void SetCurrentVFXColor(FLinearColor NewColor) {CurrentVFXColor = NewColor;}
	FLinearColor GetCurrentVFXColor() const {return CurrentVFXColor;}

	void SetCurrentElementTag(FGameplayTag NewTag) {CurrentElementTag = NewTag;}
	FGameplayTag GetCurrentElementTag() const {return CurrentElementTag;}

	void SetCurrentVFXLength(float NewLength) {CurrentVFXLength = NewLength;}
	float GetCurrentVFXLength() const {return CurrentVFXLength;}

	void SetAllowVFX(bool bAllow) {bAllowVFX = bAllow;}
	bool GetAllowVFX() const {return bAllowVFX;}
	
protected:
	UPROPERTY(Transient, Replicated)
	FLinearColor CurrentVFXColor = FLinearColor::White;
	
	UPROPERTY(Transient, Replicated)
	FGameplayTag CurrentElementTag;
	
	UPROPERTY(Transient, Replicated)
	float CurrentVFXLength = 0.f;
	
	UPROPERTY(Transient, Replicated)
	bool bAllowVFX = true;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ChatInputAction;
	
	void HandleChatInput();
};
