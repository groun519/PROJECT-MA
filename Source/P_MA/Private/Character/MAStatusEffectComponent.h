#pragma once

#include "CoreMinimal.h"
#include "Character/MAStatusEffectTypes.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "MAStatusEffectComponent.generated.h"

class AMACharacter;
class UMAAbilitySystemComponent;
class UMAImpulseComponent;

DECLARE_MULTICAST_DELEGATE(FOnStatusEffectDisplayChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UMAStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMAStatusEffectComponent();
	FOnStatusEffectDisplayChanged OnStatusEffectDisplayChanged;
	bool GetStatusEffectAnimConfig(const FGameplayTag& StatusEffectTag, FStatusEffectAnimConfig& OutConfig) const;
	void GetActiveStatusEffectDisplayEvents(TArray<FStatusEffectDisplayEvent>& OutEvents) const;
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
	void HandleStatusEffectStarted(const FStatusEffectRule& StatusEffectRule);
	void HandleStatusEffectEnded(const FStatusEffectRule& StatusEffectRule);
	void StopStatusEffectMontage(const FGameplayTag& StatusEffectTag);
	void StopAllStatusEffectMontages();
	void BeginAirborneVisual();
	void EndAirborneVisual();
	void UpdateAirborneVisual(float DeltaTime);
	FText MakeStatusEffectDisplayLabel(const FGameplayTag& StatusEffectTag) const;
	void UpdateStatusEffectDisplayState(const FGameplayEffectSpec& Spec);
	void RemoveStatusEffectDisplayState(const FGameplayTag& StatusEffectTag);

	/** Owner **/
	UPROPERTY()
	TObjectPtr<AMACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UMAAbilitySystemComponent> OwnerASC;
	
	/** Bind **/
	void BindToASC();

	/** StatusEffect Changed **/
	void HandleStatusEffectChanged(FGameplayTag Tag, int32 NewCount);

	/** StatusEffect Applied **/
	void HandleStatusEffectApplied(UAbilitySystemComponent* SourceASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle ActiveHandle);
	void HandleAirborneStatusEffectApplied(const FGameplayEffectSpec& Spec);
	void HandleImpulseStatusEffectApplied(const FGameplayEffectSpec& Spec);

	UPROPERTY(EditDefaultsOnly, Category="StatusEffect", meta=(Categories="State,Effect"))
	TMap<FGameplayTag, FStatusEffectAnimConfig> StatusEffectAnimMap;

	UPROPERTY(Transient)
	TArray<FStatusEffectRule> StatusEffectRules;

	TSet<FGameplayTag> ActiveStatusEffectTags;

	struct FStatusEffectDisplayState
	{
		FText Label;
		float Duration = 0.f;
		double EndTimeSeconds = 0.0;
	};

	TMap<FGameplayTag, FStatusEffectDisplayState> StatusEffectDisplayStates;

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
