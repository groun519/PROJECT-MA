// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShopNPC.generated.h"

class UDataTable;
class UShopWidget;
class USkeletalMeshComponent;

UCLASS()
class AShopNPC : public AActor
{
	GENERATED_BODY()
    
public: 
	AShopNPC();

protected:
	virtual void BeginPlay() override;
	
	virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Component")
	USkeletalMeshComponent* MeshComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop", meta = (AllowPrivateAccess = "true"))
	TArray<UDataTable*> ShopDataTables;
  
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UShopWidget> ShopWidgetClass;
	
	UPROPERTY()
	UShopWidget* ActiveShopWidget;
	
	void OpenShop(APlayerController* PlayerController);
};