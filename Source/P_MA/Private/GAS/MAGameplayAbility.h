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
	Sphere,
	Line
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
		FRotator BoxRot = FRotator(0, 0, 0),
		ETeamAttitude::Type TargetTeam = ETeamAttitude::Hostile,
		ETraceObjectType TraceObjType = ETraceObjectType::None,
		bool bDrawDebug = false, bool bIgnoreSelf = true);

	TArray<FHitResult> GetHitResultFromVirtualSocketTargetData(
		const FGameplayAbilityTargetDataHandle& Handle,
		ETeamAttitude::Type TargetTeam,
		bool bDrawDebug, bool bIgnoreSelf);

	UFUNCTION()
	FORCEINLINE bool ShouldDrawDebug() const { return bShouldDrawDebug; }
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShouldDrawDebug = true;

	// ===== 라인트레이스 누적을 위한 상태(첫 이벤트/이전 Base/Tip) =====
	UPROPERTY(Transient)
	bool bHasPrevSegment = false;

	UPROPERTY(Transient)
	FVector PrevBaseLocal = FVector::ZeroVector; // 이전 이벤트의 Base (로컬)

	UPROPERTY(Transient)
	FVector PrevTipLocal  = FVector::ZeroVector; // 이전 이벤트의 Tip  (로컬)
};
