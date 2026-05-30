#pragma once

#include "CoreMinimal.h"
#include "Character/MACharacter.h"
#include "InputActionValue.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MAPlayerCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class UAnimMontage;
class UCameraComponent;
class UMAInteractorComponent;
class UMACurrencyComponent;
class UReadyStateComponent;
class UReadyRideComponent;
class UReadyCheckWidgetComponent;
class UMASkillModuleInventoryComponent;
class USpringArmComponent;
class USkeletalMeshComponent;
class AMAPlayerState;

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

	/** Command **/
	UFUNCTION(Exec)
	void AddCoin(float Amount = 1000.f);
	UFUNCTION(Server, Reliable)
	void Server_AddCoin(float Amount);
	UFUNCTION(Exec)
	void RefreshShopStock();
	UFUNCTION(Server, Reliable)
	void Server_RefreshShopStock();
	UFUNCTION(Exec)
	void ShopTest();
	UFUNCTION(Server, Reliable)
	void Server_ShopTest();

	/** Ready State Component **/
	FORCEINLINE UReadyStateComponent* GetReadyStateComponent() const { return ReadyStateComponent; }

	/** Ready Ride Component **/
	FORCEINLINE UReadyRideComponent* GetReadyRideComponent() const { return ReadyRideComponent; }
	FORCEINLINE USkeletalMeshComponent* GetMountMesh() const { return MountMesh; }
	FORCEINLINE float GetRideHorizontalInput() const { return RideHorizontalInput; }

	/** Cam **/
	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	UCameraComponent* GetPlayerCamera() const { return Cam; }
	bool GetLookDirectionToMouse(FVector& OutDirection) const;

	/** Input **/
	UInputAction* GetGameplayAbilityInputAction(EMAAbilityInputID InputID) const;
	UInputMappingContext* GetGameplayInputMappingContext() const { return GameplayInputMappingContext; }
	UMAInteractorComponent* GetInteractorComponent() const { return InteractorComponent; }
	UMACurrencyComponent* GetCurrencyComponent() const { return CurrencyComponent; }
	UMASkillModuleInventoryComponent* GetSkillModuleInventoryComponent() const { return SkillModuleInventoryComponent; }
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

	UPROPERTY(VisibleDefaultsOnly, Category = "Skill")
	TObjectPtr<UMASkillModuleInventoryComponent> SkillModuleInventoryComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Currency")
	TObjectPtr<UMACurrencyComponent> CurrencyComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Interaction")
	TObjectPtr<UMAInteractorComponent> InteractorComponent;

	/** Mount **/
	UPROPERTY(VisibleDefaultsOnly, Category = "Mount")
	USkeletalMeshComponent* MountMesh;

	/** Cam **/
	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	UCameraComponent* Cam;
	
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
	TMap<EMAAbilityInputID, UInputAction*> GameplayAbilityInputActions;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* GameplayInputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	float RotationInterpSpeed = 15.f;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (ClampMin = "0.0"))
	float RotationNetSendInterval = 0.05f;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (ClampMin = "0.0"))
	float RotationNetSendYawThreshold = 1.5f;
	
	void HandleMoveInput(const FInputActionValue& InputActionValue);
	void HandleInteractInput(const FInputActionValue& InputActionValue);
	void HandleAbilityInputStarted(const FInputActionValue& InputActionValue, EMAAbilityInputID InputID);
	void HandleAbilityInputReleased(const FInputActionValue& InputActionValue, EMAAbilityInputID InputID);
	void TickHeldAbilityInputs();
	void SetAbilityInputHeld(EMAAbilityInputID InputID, bool bHeld);
	void TryActivateHeldAbilityInput(EMAAbilityInputID InputID);
	void UseInventoryItem(const FInputActionValue& InputActionValue);
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Player Rotate **/
	void UpdateRotationByReadyRide(float DeltaTime);
	void TrySendRotationToServer(const FVector& LookDirection);
	bool IsRotationBlocked() const;
	bool IsInputBlocked() const;

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

	FDelegateHandle LoadoutChangedHandle;

	UPROPERTY()
	TObjectPtr<AMAPlayerState> CachedLoadoutPlayerState;

	/** Weapon **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UWeaponComponent> WeaponComponent = nullptr;

	/** Death and Respawn **/
	virtual void OnDead() override;
	virtual void OnRespawn() override;
	void EnableInputAfterRespawnMontage();

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeadColorSaturationScale = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TObjectPtr<UNiagaraSystem> RespawnVFX = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TObjectPtr<UAnimMontage> RespawnMontage = nullptr;

	FTimerHandle RespawnInputEnableTimerHandle;

	/** MiniMap **/
	UPROPERTY(VisibleAnywhere, Category="MinimapCamera")
	USpringArmComponent* MinimapCameraBoom;

	UPROPERTY(VisibleAnywhere, Category="MinimapCamera")
	USceneCaptureComponent2D* MinimapCapture;

	UPROPERTY(EditDefaultsOnly, Category="MinimapCamera", meta=(ClampMin="0.01"))
	float MinimapCaptureInterval = 0.05f;
	float MinimapCaptureAccumulatedTime = 0.f;

	UPROPERTY(VisibleAnywhere, Category="MinimapCamera")
	class UPaperSpriteComponent* MinimapSprite;

	void InitializeMinimapCapture();
	void TickMinimapCapture(float DeltaTime);

	/** Inventory **/
	class UInventoryComponent* InventoryComponent;

	TSet<EMAAbilityInputID> HeldAbilityInputIDs;
	UWeaponComponent* GetWeaponComponent() const { return WeaponComponent; }
};
