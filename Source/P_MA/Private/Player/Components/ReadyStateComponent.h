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
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoopReadyStateChanged, bool /*bIsReady*/);

	UReadyStateComponent();

	/** Ready by Montage **/
	void ReadyAndMoveIn(FVector InDir, float MovingUnit);

	UFUNCTION(Server, Reliable)
	void ServerReadyAndMoveIn(FVector InDir, float MovingUnit);
	
	void SetReady(bool bNewReady);
	FORCEINLINE bool IsReady() const { return bIsReady; }

	void SetLoopReady(bool bNewReady);
	FORCEINLINE bool IsLoopReady() const { return bIsLoopReady; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);

	UFUNCTION(Server, Reliable)
	void ServerSetLoopReady(bool bNewReady);

	FOnReadyStateChanged OnReadyStateChanged;
	FOnLoopReadyStateChanged OnLoopReadyStateChanged;

private:
	void HandleReadyStateChanged();
	void HandleLoopReadyStateChanged();

	UPROPERTY(ReplicatedUsing=OnRep_IsReady)
	bool bIsReady = false;

	UPROPERTY(ReplicatedUsing=OnRep_IsLoopReady)
	bool bIsLoopReady = false;

	UFUNCTION()
	void OnRep_IsReady();

	UFUNCTION()
	void OnRep_IsLoopReady();
};
