#pragma once

#include "CoreMinimal.h"
#include "Character/MACharacter.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "Player/Loadout/LoadoutTypes.h"
#include "MAPlayerCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class UAnimMontage;
class UMACameraComponent;
class UMAInventoryComponent;
class UMAInteractorComponent;
class UMACurrencyComponent;
class UReadyStateComponent;
class UReadyRideComponent;
class UReadyCheckWidgetComponent;
class USpringArmComponent;
class USkeletalMeshComponent;
class ULoadoutComponent;
class AMAPlayerState;
class AMAReviveActor;

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

	/** Cheat **/
	UFUNCTION(Server, Reliable)
	void Server_AddCoin(float Amount);
	UFUNCTION(Server, Reliable)
	void Server_RefreshShopStock();
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
	UMACameraComponent* GetPlayerCamera() const { return Cam; }
	bool GetLookDirectionToMouse(FVector& OutDirection) const;

	/** Input **/
	UInputAction* GetGameplayAbilityInputAction(FGameplayTag SlotTag) const;
	UInputMappingContext* GetGameplayInputMappingContext() const { return GameplayInputMappingContext; }
	UMAInteractorComponent* GetInteractorComponent() const { return InteractorComponent; }
	UMACurrencyComponent* GetCurrencyComponent() const { return CurrencyComponent; }
	UMAInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "Loadout")
	TObjectPtr<ULoadoutComponent> LoadoutComponent;

	/** Ready State Component **/
	UPROPERTY(VisibleDefaultsOnly, Category = "Ready")
	UReadyStateComponent* ReadyStateComponent;

	/** Ready Ride Component **/
	UPROPERTY(VisibleDefaultsOnly, Category = "Ready")
	UReadyRideComponent* ReadyRideComponent;

	/** Ready Check Widget **/
	UPROPERTY(VisibleDefaultsOnly, Category = "UI")
	UReadyCheckWidgetComponent* ReadyCheckWidget;

	UPROPERTY(VisibleDefaultsOnly, Category="Inventory")
	TObjectPtr<UMAInventoryComponent> InventoryComponent;

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
	UMACameraComponent* Cam;
	
	FVector GetMoveForwardDir() const; 
	FVector GetMoveRightDir() const;

	/** Input **/
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* InteractInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TMap<FGameplayTag, UInputAction*> GameplayAbilityInputActions;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* GameplayInputMappingContext;

protected:
	/** Optional minimap work that non-combat player variants can disable. */
	bool bEnableMinimapCapture = true;

private:

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	float RotationInterpSpeed = 15.f;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (ClampMin = "0.0"))
	float RotationNetSendInterval = 0.05f;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (ClampMin = "0.0"))
	float RotationNetSendYawThreshold = 1.5f;
	
	void HandleMoveInput(const FInputActionValue& InputActionValue);
	void HandleInteractInput(const FInputActionValue& InputActionValue);
	void HandleAbilityInputStarted(const FInputActionValue& InputActionValue, FGameplayTag SlotTag);
	void HandleAbilityInputReleased(const FInputActionValue& InputActionValue, FGameplayTag SlotTag);
	void TickHeldAbilityInputs();
	void SetAbilityInputHeld(FGameplayTag SlotTag, bool bHeld);
	void TryActivateHeldAbilityInput(FGameplayTag SlotTag);
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
public:
	void RespawnImmediately();

protected:
	virtual void DeathTagUpdated(const FGameplayTag Tag, int32 NewCount) override;
	virtual void OnDead() override;

private:
	void Respawn();
	void FinishRespawn();

public:
	UPROPERTY(EditDefaultsOnly, Category = "Death|Revive")
	TSubclassOf<AMAReviveActor> ReviveActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeadColorSaturationScale = 0.25f;

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

	TSet<FGameplayTag> HeldAbilitySlotTags;
	UWeaponComponent* GetWeaponComponent() const { return WeaponComponent; }

private:
	UPROPERTY(Transient)
	TObjectPtr<AMAReviveActor> ActiveReviveActor = nullptr;

	void SpawnReviveActor();
	void ClearReviveActor();
};
