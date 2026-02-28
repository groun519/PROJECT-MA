// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Framework/MAGameStateTypes.h"
#include "EnvironmentManager.generated.h"

class UDataTable;
class UPCGGraph;

class AMAGameMode;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnvironmentTagChanged, const FGameplayTag&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnvironmentPCGChanged, UPCGGraph*);

UCLASS()
class P_MA_API AEnvironmentManager : public AActor
{
	GENERATED_BODY()

public:
	AEnvironmentManager();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	static AEnvironmentManager* FindEnvironmentManager(UWorld* InWorld);

	UFUNCTION(BlueprintCallable, Category = "Environment")
	bool SetCurrentEnvTag(FGameplayTag InEnvTag);

	UFUNCTION(BlueprintCallable, Category = "Environment")
	void BroadcastCurrentEnvironment();

	UFUNCTION(BlueprintPure, Category = "Environment")
	FGameplayTag GetCurrentEnvTag() const { return CurrentEnvTag; }

	UFUNCTION(BlueprintPure, Category = "Environment")
	EMASectorState GetMASectorState() const { return CachedMASectorState; }

	FOnEnvironmentTagChanged OnEnvironmentTagChanged;
	FOnEnvironmentPCGChanged OnEnvironmentPCGChanged;

private:
	bool InitCachedMAGameMode();
	bool InitStageManager();
	UPCGGraph* FindPCGGraphByTag(FGameplayTag InEnvTag) const;
	bool PickRandomDifferentEnvTag(FGameplayTag& OutEnvTag) const;
	void OnHandleSectorStateChanged(EMASectorState NewState);
	void OnHandleStageChangeEnvRequested();

	UPROPERTY()
	AMAGameMode* CachedMAGameMode = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Environment")
	EMASectorState CachedMASectorState = EMASectorState::Wait;

	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_CurrentEnvTag, Category = "Environment")
	FGameplayTag CurrentEnvTag;

	UPROPERTY(VisibleAnywhere, Category = "Environment")
	FGameplayTag PreviousEnvTag;

	UPROPERTY(EditAnywhere, Category = "Environment")
	UDataTable* EnvironmentDataTable = nullptr;

	UFUNCTION()
	void OnRep_CurrentEnvTag();
};
