#pragma once

#include "CoreMinimal.h"
#include "Framework/MAGameStateTypes.h"
#include "NPC/MANPC.h"
#include "Player/Camera/MACameraTypes.h"
#include "Shop/MAShopTypes.h"
#include "MAShopNPC.generated.h"

class AMAPlayerCharacter;
class UMAModuleQualityData;
class UMASkillModulePool;
class UMASkillModule;
class UMAShopWidget;
class UCameraComponent;
class USpotLightComponent;

UCLASS()
class P_MA_API AMAShopNPC : public AMANPC
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	AMAShopNPC();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void CloseShop(APlayerController* PlayerController);
	const TArray<FMAShopProduct>& GetCurrentProducts() const { return CurrentProducts; }
	bool Purchase(APlayerController* PlayerController, int32 StockId);
	void RefreshStock();
	void SetStockCountsForTest(int32 Count);

private:
	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UCameraComponent> ShopCameraComponent;

	UPROPERTY(EditDefaultsOnly, Category="Shop")
	TSubclassOf<UMAShopWidget> ShopWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="Shop|Product")
	TObjectPtr<UMASkillModulePool> ModulePool;

	UPROPERTY(EditDefaultsOnly, Category="Shop|Stock")
	FMAShopStockCountRange ModuleStockCountRange;

	UPROPERTY(EditDefaultsOnly, Category="Shop|Stock")
	FMAShopStockCountRange ItemStockCountRange;

	// TODO: Test-only stock refresh hook. Consider removing when the map/shop flow is finalized.
	UPROPERTY(EditDefaultsOnly, Category="Shop|Stock")
	EMASectorState RefreshStockState = EMASectorState::Loop;

	// TODO: Test-only shop visibility hook. Consider removing when the map/shop flow is finalized.
	UPROPERTY(EditDefaultsOnly, Category="Shop|Temporary")
	EMASectorState TemporaryVisibleState = EMASectorState::EndBattle;

	UPROPERTY(EditDefaultsOnly, Category="Shop|Price")
	TObjectPtr<UMAModuleQualityData> ModuleQualityData;

	UPROPERTY(EditDefaultsOnly, Category="Shop|Camera")
	FMACameraFadeSettings ShopCameraFadeSettings;

	UPROPERTY(EditDefaultsOnly, Category="Shop|Camera")
	FMACameraPresentationSettings PresentationSettings;

	UPROPERTY(Transient)
	TObjectPtr<USpotLightComponent> PresentationFillLight;
	
	void HandleInteract(AMAPlayerCharacter* Interactor);
	void OpenShopFor(AMAPlayerCharacter* Interactor);
	void EnterShopPresentation(APlayerController& PlayerController);
	void ExitShopPresentation(APlayerController& PlayerController);
	void BeginPresentationTransition(APlayerController& PlayerController, bool bEntering);
	void ApplyPresentationState(APlayerController& PlayerController, bool bEntering);
	void HandleSectorStateChanged(EMASectorState NewState);
	void SetTemporaryShopVisible(bool bVisible);
	TArray<FMAShopProduct> GenerateStock() const;
	int32 ResolvePrice(const UMASkillModule* Module) const;

	UFUNCTION()
	void OnRep_CurrentProducts();

	UFUNCTION()
	void OnRep_TemporaryShopVisible();

	UPROPERTY(Transient)
	TObjectPtr<UMAShopWidget> ActiveShopWidget = nullptr;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_CurrentProducts)
	TArray<FMAShopProduct> CurrentProducts;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_TemporaryShopVisible)
	bool bTemporaryShopVisible = false;

	UPROPERTY(Transient)
	TWeakObjectPtr<AMAPlayerCharacter> HiddenShopInteractor;

	TWeakObjectPtr<APlayerController> FadingPlayerController;
	FTimerHandle ShopCameraFadeTimerHandle;
};
