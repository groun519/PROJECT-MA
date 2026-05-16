#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/Camera/MACameraTypes.h"
#include "Shop/MAShopTypes.h"
#include "MAShopNPC.generated.h"

class AMAPlayerCharacter;
class UMAHighlightComponent;
class UMASkillDefinition;
class UMAShopWidget;
class UCameraComponent;
class UMAInteractableComponent;
class USkeletalMeshComponent;

UCLASS()
class P_MA_API AMAShopNPC : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
public:
	AMAShopNPC();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void CloseShop(APlayerController* PlayerController);
	const TArray<FMAShopStockEntry>& GetCurrentStockEntries() const { return CurrentStockEntries; }
	bool RequestPurchase(APlayerController* PlayerController, int32 StockId);
	void RefreshStock();

private:
	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UCameraComponent> ShopCameraComponent;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UMAInteractableComponent> InteractableComponent;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UMAHighlightComponent> HighlightComponent;

	UPROPERTY(EditDefaultsOnly, Category="Shop")
	TSubclassOf<UMAShopWidget> ShopWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="Shop|Product", meta=(ContentDir))
	TArray<FDirectoryPath> ModuleRootPaths;

	UPROPERTY(EditDefaultsOnly, Category="Shop|Stock")
	FMAShopStockCountRange ModuleStockCountRange;

	UPROPERTY(EditDefaultsOnly, Category="Shop|Camera")
	FMACameraFadeSettings ShopCameraFadeSettings;
	
	void HandleInteract(AMAPlayerCharacter* Interactor);
	void OpenShopFor(AMAPlayerCharacter* Interactor);
	TArray<FMAShopStockEntry> GenerateShopStock() const;

	UFUNCTION()
	void OnRep_CurrentStockEntries();

	UPROPERTY(Transient)
	TObjectPtr<UMAShopWidget> ActiveShopWidget = nullptr;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_CurrentStockEntries)
	TArray<FMAShopStockEntry> CurrentStockEntries;

	UPROPERTY(Transient)
	TWeakObjectPtr<AMAPlayerCharacter> HiddenShopInteractor;
};

