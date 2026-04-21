#pragma once

#include "CoreMinimal.h"
#include "DebugShapeHelper.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GameplayTagContainer.h"
#include "MASkillAction_MeleeOverlapShape.generated.h"

class AActor;

USTRUCT(BlueprintType)
struct FMASkillActionConfig_MeleeOverlapShape
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Shape")
	EVA_Shape Shape = EVA_Shape::Circle;

	UPROPERTY(EditDefaultsOnly, Category="Shape", meta=(ClampMin="0.0"))
	float SphereRadius = 100.f;

	UPROPERTY(EditDefaultsOnly, Category="Shape")
	FVector BoxHalfSize = FVector(100.f, 50.f, 50.f);

	UPROPERTY(EditDefaultsOnly, Category="Shape")
	FVector LocalOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category="Shape")
	FRotator LocalRotation = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, Category="Shape")
	bool bUseSector = false;

	UPROPERTY(EditDefaultsOnly, Category="Shape", meta=(ClampMin="0.0", ClampMax="360.0"))
	float SectorAngle = 90.f;

	UPROPERTY(EditDefaultsOnly, Category="Shape")
	bool bIgnoreOwner = true;

	UPROPERTY(EditDefaultsOnly, Category="Debug")
	bool bDrawDebug = false;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Melee Overlap Shape")
class P_MA_API UMASkillAction_MeleeOverlapShape : public UMASkillAction
{
	GENERATED_BODY()

public:
	virtual void ResetRuntimeState() override { IgnoredActors.Reset(); }
	virtual void Execute(UMASkillAbility& OwnerAbility, const FGameplayEventData& Payload) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Action")
	FMASkillActionConfig_MeleeOverlapShape Config;

	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(Categories="Damage"))
	FGameplayTag DamagePayloadTag;

	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> IgnoredActors;
};
