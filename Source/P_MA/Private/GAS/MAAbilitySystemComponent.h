#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "MAAbilitySystemComponent.generated.h"

struct FGameplayEffectModCallbackData;
class UAnimMontage;
class UGameplayAbility;

/**
 * Executed Gameplay Cue Batch
 *
 * 동일한 Unreliable Multicast RPC가 한 net update에서 제한되는 문제를 막기 위해,
 * 실행 Cue를 현재 프레임 동안 수집하고 네트워크 TickFlush 직전에 한 번 전송한다.
 */
USTRUCT()
struct FMAGameplayCueBatchEntry
{
	GENERATED_BODY()

	FMAGameplayCueBatchEntry() = default;
	FMAGameplayCueBatchEntry(
		const FGameplayTagContainer& InGameplayCueTags,
		const FGameplayCueParameters& InParameters)
		: GameplayCueTags(InGameplayCueTags)
		, Parameters(InParameters)
	{
	}

	UPROPERTY()
	FGameplayTagContainer GameplayCueTags;

	UPROPERTY()
	FGameplayCueParameters Parameters;
};
/****/

UCLASS()
class UMAAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UMAAbilitySystemComponent();
	void InitializeBaseAttributes();
	void ServerSideInit();
	void ApplyFullStatEffect();
	void ApplyReviveStatEffect();
	void NotifyDamageAppliedFromGameplayEffect(const FGameplayEffectModCallbackData& Data);

	/** Executed Gameplay Cue Batch **/
	void ExecuteGameplayCues(
		const FGameplayTagContainer& GameplayCueTags,
		const FGameplayCueParameters& Parameters);
	/****/

	float PlayMontageWithBlendIn(
		UGameplayAbility* AnimatingAbility,
		FGameplayAbilityActivationInfo ActivationInfo,
		UAnimMontage* Montage,
		float PlayRate,
		FName StartSectionName,
		float BlendInTime);
	virtual float PlayMontageSimulated(
		UAnimMontage* Montage,
		float PlayRate,
		FName StartSectionName = NAME_None) override;

	UPROPERTY(Transient)
	FGameplayTagContainer AppliedBaseTags;
	
private:
	/** Executed Gameplay Cue Batch **/
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void FlushPendingGameplayCues(float);

	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticast_ExecuteGameplayCueBatch(const TArray<FMAGameplayCueBatchEntry>& Entries);

	UPROPERTY(Transient)
	TArray<FMAGameplayCueBatchEntry> PendingGameplayCueEntries;

	FDelegateHandle GameplayCueFlushHandle;
	/****/

	void GiveInitialAbilities();
	void AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int Level = 1);
	void HealthUpdated(const FOnAttributeChangeData& ChangeData);
	void ShowDamageText(const FGameplayEffectModCallbackData& Data, bool bIsIncoming) const;
	void ApplyMontageBlendIn(UAnimMontage* Montage, float PlayRate, float BlendInTime) const;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TArray<TSubclassOf<UGameplayAbility>> Abilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TArray<TSubclassOf<UGameplayAbility>> BasicAbilities;
};
