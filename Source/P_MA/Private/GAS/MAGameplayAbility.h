#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GenericTeamAgentInterface.h"
#include "MAGameplayAbility.generated.h"

UCLASS()
class UMAGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UMAGameplayAbility();

	UAnimInstance* GetOwnerAnimInstance() const;
	TArray<FHitResult> GetHitResultsFromAreaTargetData(const FGameplayAbilityTargetDataHandle& Handle);
	TArray<FHitResult> GetHitResultsFromAreaTargetData(const FGameplayAbilityTargetDataHandle& Handle, int32 OverrideTargetRelationMask);
	FGameplayEffectSpecHandle MakeDamageEffectSpec(
		TSubclassOf<UGameplayEffect> GameplayEffect,
		int32 Level = 1,
		const FMADamageExecutionConfig* DamageConfig = nullptr);
	void ApplyGameplayEffectSpecToHitResultActor(
		const FHitResult& HitResult,
		const FGameplayEffectSpecHandle& EffectSpecHandle);
	void ApplyGameplayEffectToHitResultActor(
		const FHitResult& HitResult,
		TSubclassOf<UGameplayEffect> GameplayEffect,
		int Level = 1,
		const FMADamageExecutionConfig* DamageConfig = nullptr);
	FGenericTeamId GetOwnerTeamId() const;

protected:
	ACharacter* GetOwningAvatarCharacter();
	void StopMontageAfterCurrentSection(UAnimMontage* Montage);
	void PlayMontageLocally(UAnimMontage* Montage);

private:
	UPROPERTY()
	ACharacter* AvatarCharacter;
};
