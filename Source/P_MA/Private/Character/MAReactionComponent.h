#pragma once

#include "CoreMinimal.h"
#include "Character/MAReactionTypes.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "MAReactionComponent.generated.h"

class AMACharacter;
class UMAAbilitySystemComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UMAReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMAReactionComponent();
	bool GetReactionAnimConfig(const FGameplayTag& ReactionTag, FReactionAnimConfig& OutConfig) const;
	void ResetTransientReactionState();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void BuildReactionRules();
	const FReactionRule* FindReactionRule(const FGameplayTag& ReactionTag) const;
	bool HasActiveImpulseReaction() const;
	void HandleCrowdControlStarted(const FReactionRule& ReactionRule);
	void HandleCrowdControlEnded(const FReactionRule& ReactionRule);
	void StopReactionMontage(const FGameplayTag& ReactionTag);
	void StopAllReactionMontages();
	void ClearImpulseReactionState();
	void RecalculateImpulseReactionVelocity(bool bStopMovementImmediately);
	void RefreshControlBlockTags();

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
	void HandleImpulseCrowdControlApplied(const FGameplayEffectSpec& Spec);

	/** Impulse **/
	void ApplyImpulseReaction(EReactionImpulseMode ImpulseMode, float Magnitude, const FVector& SourcePoint, const FGameplayTag& ReactionTag);
	void BeginImpulseMovementOverride();
	void EndImpulseMovementOverride();
	FVector GetReactionDirection(const FVector& SourcePoint, EReactionImpulseMode ImpulseMode) const;

	UPROPERTY(EditDefaultsOnly, Category="Reaction", meta=(Categories="State,Effect"))
	TMap<FGameplayTag, FReactionAnimConfig> ReactionAnimMap;

	UPROPERTY(Transient)
	TArray<FReactionRule> ReactionRules;

	TSet<FGameplayTag> ActiveCrowdControlTags;
	float SavedImpulseGroundFriction = 0.f;
	float SavedImpulseBrakingFrictionFactor = 0.f;
	float SavedImpulseBrakingDecelerationWalking = 0.f;
	TEnumAsByte<ECollisionResponse> SavedImpulseHitboxResponse = ECR_Block;
	bool bImpulseMovementOverrideActive = false;
	TMap<FGameplayTag, FVector> ActiveImpulseContributions;
};
