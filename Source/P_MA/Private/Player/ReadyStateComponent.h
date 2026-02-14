// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReadyStateComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MA_API UReadyStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnReadyStateChanged, bool /*bIsReady*/);

	UReadyStateComponent();

	/** Ready by Montage **/
	void ReadyAndMoveIn(FVector InDir, float MovingUnit);

	UFUNCTION(Server, Reliable)
	void ServerReadyAndMoveIn(FVector InDir, float MovingUnit);
	
	void SetReady(bool bNewReady);
	FORCEINLINE bool IsReady() const { return bIsReady; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);

	FOnReadyStateChanged OnReadyStateChanged;

private:
	void HandleReadyStateChanged();

	UPROPERTY(ReplicatedUsing=OnRep_IsReady)
	bool bIsReady = false;

	UFUNCTION()
	void OnRep_IsReady();
};
