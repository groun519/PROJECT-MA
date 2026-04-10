#pragma once

#include "CoreMinimal.h"
#include "Character/MAStatusEffectTypes.h"
#include "Components/ActorComponent.h"
#include "MAImpulseComponent.generated.h"

class AMACharacter;
class UCharacterMovementComponent;
class UCapsuleComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UMAImpulseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMAImpulseComponent();

	void ApplyStatusEffectImpulse(EStatusEffectImpulseMode ImpulseMode, float Magnitude, const FVector& SourcePoint, const FGameplayTag& StatusEffectTag);
	void ApplyActionImpulseVelocity(UObject* OwnerObject, const FGameplayTag& ImpulseTag, const FVector& Velocity, float Duration, bool bStopMovementImmediately = true);
	void RemoveImpulse(const FGameplayTag& StatusEffectTag);
	void StopOwnedActionImpulses(UObject* OwnerObject);
	void CancelInterruptibleActionImpulses();
	void ResetImpulseState();

protected:
	virtual void BeginPlay() override;

private:
	struct FActionImpulseOwner
	{
		TWeakObjectPtr<UObject> Object = nullptr;

		bool Matches(UObject* InObject) const
		{
			return Object.Get() == InObject;
		}
	};

	AMACharacter* ResolveOwnerCharacter();
	UCharacterMovementComponent* GetOwnerCharacterMovement() const;
	UCapsuleComponent* GetOwnerCapsule() const;
	void ApplyImpulseVelocityInternal(const FGameplayTag& ImpulseTag, const FVector& Velocity, bool bStopMovementImmediately);
	void ScheduleActionImpulseStop(const FGameplayTag& ImpulseTag, float Duration);
	void ClearActionImpulseTimer(const FGameplayTag& ImpulseTag);
	void ClearAllActionImpulseTimers();
	bool HasActiveImpulse() const;
	void BeginImpulseMovementOverride();
	void EndImpulseMovementOverride();
	void ClearImpulseState();
	void RecalculateImpulseVelocity(bool bStopMovementImmediately);
	FVector GetStatusEffectDirection(const FVector& SourcePoint, EStatusEffectImpulseMode ImpulseMode) const;

	UPROPERTY()
	TObjectPtr<AMACharacter> OwnerCharacter;

	float SavedImpulseGroundFriction = 0.f;
	float SavedImpulseBrakingFrictionFactor = 0.f;
	float SavedImpulseBrakingDecelerationWalking = 0.f;
	TEnumAsByte<ECollisionResponse> SavedImpulseHitboxResponse = ECR_Block;
	bool bImpulseMovementOverrideActive = false;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FVector> ActiveImpulseContributions;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FTimerHandle> ActiveActionImpulseTimers;

	TMap<FGameplayTag, FActionImpulseOwner> ActiveActionImpulseOwners;
};
