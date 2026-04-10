#pragma once

#include "CoreMinimal.h"
#include "Character/MAStatusEffectTypes.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "MAStatusEffectComponent.generated.h"

class AMACharacter;
class UMAAbilitySystemComponent;
class UMAImpulseComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UMAStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMAStatusEffectComponent();
	bool GetStatusEffectAnimConfig(const FGameplayTag& StatusEffectTag, FStatusEffectAnimConfig& OutConfig) const;
	void PlayReplicatedStatusEffectImpulse(const FGameplayTag& StatusEffectTag, float Magnitude, const FVector& SourcePoint);
	void ResetTransientStatusEffectState();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	const FStatusEffectRule* FindStatusEffectRule(const FGameplayTag& StatusEffectTag) const;
	UMAImpulseComponent* GetImpulseComponent() const;
	void ApplyStatusEffectImpulse(const FStatusEffectRule& StatusEffectRule, float Magnitude, const FVector& SourcePoint);
	void HandleCrowdControlStarted(const FStatusEffectRule& StatusEffectRule);
	void HandleCrowdControlEnded(const FStatusEffectRule& StatusEffectRule);
	void StopStatusEffectMontage(const FGameplayTag& StatusEffectTag);
	void StopAllStatusEffectMontages();
	void BeginAirborneVisual();
	void EndAirborneVisual();
	void UpdateAirborneVisual(float DeltaTime);

	/** Owner **/
	UPROPERTY()
	TObjectPtr<AMACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UMAAbilitySystemComponent> OwnerASC;
	
	/** Bind **/
	void BindToASC();

	/** CrowdControl Changed **/
	void HandleCrowdControlChanged(FGameplayTag Tag, int32 NewCount);

	/** CrowdControl Applied **/
	void HandleCrowdControlApplied(UAbilitySystemComponent* SourceASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle ActiveHandle);
	void HandleAirborneCrowdControlApplied(const FGameplayEffectSpec& Spec);
	void HandleImpulseCrowdControlApplied(const FGameplayEffectSpec& Spec);

	UPROPERTY(EditDefaultsOnly, Category="StatusEffect", meta=(Categories="State,Effect"))
	TMap<FGameplayTag, FStatusEffectAnimConfig> StatusEffectAnimMap;

	UPROPERTY(Transient)
	TArray<FStatusEffectRule> StatusEffectRules;

	TSet<FGameplayTag> ActiveCrowdControlTags;

	/** Airborne Visual **/
	UPROPERTY(EditDefaultsOnly, Category="StatusEffect|Airborne", meta=(ClampMin="0.0"))
	float AirborneVisualHeight = 100.f;

	bool bAirborneVisualActive = false;
	float CurrentAirborneVisualHeight = 0.f;
	float CurrentAirborneVisualDuration = 0.f;
	float CurrentAirborneVisualRiseTime = 0.f;
	float CurrentAirborneVisualElapsedTime = 0.f;
	FVector BaseAirborneMeshRelativeLocation = FVector::ZeroVector;
	FVector AirborneVisualStartMeshRelativeLocation = FVector::ZeroVector;
	FVector AirborneVisualPeakMeshRelativeLocation = FVector::ZeroVector;
};
