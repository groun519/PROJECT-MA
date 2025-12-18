// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "InteractComponent.generated.h"

class AMAPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractRequested, AMAPlayerCharacter*, Interactor);

UCLASS()
class P_MA_API UInteractComponent : public USphereComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void OnRegister() override;

public:
	UInteractComponent();

	UPROPERTY(BlueprintAssignable, Category="MA|Interact")
	FOnInteractRequested OnInteractRequested;

	UFUNCTION(BlueprintCallable, Category="MA|Interact")
	void RequestInteract(AMAPlayerCharacter* Interactor);
	
	void SetActive(bool bNewActive, AMAPlayerCharacter* Interactor);

	void ShowInteractKeyUI(AMAPlayerCharacter* Interactor);
	void HideInteractKeyUI();

	UPROPERTY(Transient)
	bool bActive = false;

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleDefaultsOnly, Category = "UI")
	class UWidgetComponent* InteractKeyWidgetComp;
};
