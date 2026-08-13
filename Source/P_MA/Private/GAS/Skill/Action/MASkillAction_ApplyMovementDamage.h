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
		const FMAResolvedDamage& InResolvedDamage,
		float InCapsuleRadiusMultiplier,
		float InCapsuleHalfHeightMultiplier,
		bool bInDrawTrailDecal,
		float InMinTrailDecalDistance);

private:
	void SweepMovement(const FVector& Start, const FVector& End);
	void SpawnTrailDecal(const FVector& Start, const FVector& End, float Radius) const;

	TWeakObjectPtr<UMASkillAbility> OwnerAbility;
	TWeakObjectPtr<AMACharacter> OwnerCharacter;
	TWeakObjectPtr<UMAImpulseComponent> ImpulseComponent;
	FMAActionImpulseHandle MovementHandle;
	FMAResolvedDamage ResolvedDamage;
	FGameplayTag VisualElementTag;
	TSet<TWeakObjectPtr<AActor>> HitActors;
	FVector PreviousLocation = FVector::ZeroVector;
	float CapsuleRadiusScale = 1.f;
	float CapsuleHalfHeightScale = 1.f;
	bool bDrawTrailDecal = true;
	float MinTrailDecalDistance = 80.f;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillAction_ApplyMovementDamage : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_ApplyMovementDamage() { SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub; }

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(Categories="Damage"))
	FGameplayTag DamagePayloadTag;

	UPROPERTY(EditDefaultsOnly, Category="Collision", meta=(ClampMin="0.01", DisplayName="Capsule Radius Multiplier"))
	float CapsuleRadiusScale = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Collision", meta=(ClampMin="0.01", DisplayName="Capsule Half Height Multiplier"))
	float CapsuleHalfHeightScale = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Visual")
	bool bDrawTrailDecal = true;

	UPROPERTY(EditDefaultsOnly, Category="Visual", meta=(ClampMin="0.0", EditCondition="bDrawTrailDecal", EditConditionHides))
	float MinTrailDecalDistance = 80.f;
};
