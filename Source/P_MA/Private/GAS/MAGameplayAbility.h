// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GenericTeamAgentInterface.h"
#include "MAGameplayAbility.generated.h"

UENUM()
enum class ETraceObjectType : uint8
{
	None,
	Box,
	Sphere
};

/**
 * 
 */
UCLASS()
class UMAGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	class UAnimInstance* GetOwnerAnimInstance() const;
	TArray<FHitResult> GetHitResultFromSweepLocationTargetData(
		const FGameplayAbilityTargetDataHandle& TargetDataHandle,
		FVector HalfSize = FVector(30.f, 0, 0),
		ETeamAttitude::Type TargetTeam = ETeamAttitude::Hostile,
		ETraceObjectType TraceObjType,
		bool bDrawDebug = false, bool bIgnoreSelf = true) const;
	TArray<FHitResult> GetHitResultFromSweepLocationTargetData_Box(
		const FGameplayAbilityTargetDataHandle& TargetDataHandle,
		FVector HalfSize = FVector(30.f, 0, 0),
		ETeamAttitude::Type TargetTeam = ETeamAttitude::Hostile,
		bool bDrawDebug = false, bool bIgnoreSelf = true) const;
	TArray<FHitResult> GetHitResultFromSweepLocationTargetData_Sphere(
		const FGameplayAbilityTargetDataHandle& TargetDataHandle,
		float Radius,
		ETeamAttitude::Type TargetTeam = ETeamAttitude::Hostile,
		bool bDrawDebug = false, bool bIgnoreSelf = true) const;
	

	UFUNCTION()
	FORCEINLINE bool ShouldDrawDebug() const { return bShouldDrawDebug; }
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShouldDrawDebug = false;
};
