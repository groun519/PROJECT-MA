#pragma once

#include "CoreMinimal.h"
#include "Character/MAImpulseComponent.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GameFramework/Actor.h"
#include "MASkillAction_ApplyMovementDamage.generated.h"

class AMACharacter;
class UMAImpulseComponent;
class UMASkillAbility;

UCLASS()
class P_MA_API AMASkillMovementDamageRuntime : public AActor
{
	GENERATED_BODY()

public:
	AMASkillMovementDamageRuntime();
	virtual void Tick(float DeltaSeconds) override;

	bool Initialize(
		UMASkillAbility& InOwnerAbility,
		const FMASkillScopes& InScopes,
		const FMAActionImpulseHandle& InMovementHandle,
		const FResolvedSkillDamage& InResolvedDamage,
		float InCapsuleRadiusScale,
		float InCapsuleHalfHeightScale);

private:
	void SweepMovement(const FVector& Start, const FVector& End);

	TWeakObjectPtr<UMASkillAbility> OwnerAbility;
	TWeakObjectPtr<AMACharacter> OwnerCharacter;
	TWeakObjectPtr<UMAImpulseComponent> ImpulseComponent;
	FMASkillScopes Scopes;
	FMAActionImpulseHandle MovementHandle;
	FResolvedSkillDamage ResolvedDamage;
	TSet<TWeakObjectPtr<AActor>> HitActors;
	FVector PreviousLocation = FVector::ZeroVector;
	float CapsuleRadiusScale = 1.f;
	float CapsuleHalfHeightScale = 1.f;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillAction_ApplyMovementDamage : public UMASkillAction
{
	GENERATED_BODY()

public:
	virtual void Execute(
		UMASkillAbility& OwnerAbility,
		const FMASkillEvent& Event,
		const FMASkillScopes& Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(Categories="Damage"))
	FGameplayTag DamagePayloadTag;

	UPROPERTY(EditDefaultsOnly, Category="Collision", meta=(ClampMin="0.01"))
	float CapsuleRadiusScale = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Collision", meta=(ClampMin="0.01"))
	float CapsuleHalfHeightScale = 1.f;
};
