#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MAAbilitySystemStatics.generated.h"


class UGameplayAbility;
struct FGameplayAbilitySpec;
class UAbilitySystemComponent;
enum class EMADamageAttributeSide : uint8;
struct FGameplayAttribute;
struct FMADamageExecutionConfig;
/**
 * 
 */
UCLASS()
class UMAAbilitySystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static FGameplayTag GetDeadStatTag();
	static FGameplayTag GetStunStatTag();
	static FGameplayTag GetFrozenStatTag();
	static FGameplayTag GetRootStatTag();
	static FGameplayTag GetAirborneStatTag();
	static FGameplayTag GetAirborneRiseTimeTag();
	static FGameplayTag GetGrabStatTag();
	static FGameplayTag GetStaggerStatTag();
	static FGameplayTag GetKnockbackStatTag();
	
	static FGameplayTag GetRotationLockTag();
	static FGameplayTag GetInputBlockTag();
	static FGameplayTag GetMoveBlockTag();
	static FGameplayTag GetAbilityBlockTag();
	static FGameplayTag GetReactionSourceXTag();
	static FGameplayTag GetReactionSourceYTag();
	static FGameplayTag GetReactionSourceZTag();

	static FGameplayTag GetHealthFullStatTag();
	static FGameplayTag GetHealthEmptyStatTag();

	static FGameplayTag GetDefaultVisualElementTag();
	static FGameplayTag GetPlayerRespawnGameplayCueTag();
	
	static FGameplayTag GetBehaviorMultiplierTag();
	static FGameplayTag GetDamageBaseTag();
	static FGameplayTag GetAppliedDamageTag();
	static FGameplayTag GetDamageTargetTag();
	static FGameplayTag GetFinalDamageMultiplierTag();
	static FGameplayTag GetDamageVarianceTag();
	static FGameplayTag GetSkillAttackSpeedMultiplierTag();
	static FGameplayTag GetSkillFocusOffsetTag();
	static FGameplayTag GetSkillAreaScaleTag();
	static FGameplayTag GetSkillChargeRatioTag();
	static FGameplayTag GetHitEventTag();
	static FGameplayTag GetMovementStartEventTag();
	static FGameplayTag GetChargeCompletedEventTag();
	static FGameplayTag GetMovementHandleTag();
	static FGameplayTag GetDefaultDamageTypeTag();
	static FGameplayTag GetHealDamageTypeTag();
	static FGameplayTag GetFireDamageTypeTag();
	static FGameplayTag GetIceDamageTypeTag();
	static FGameplayTag GetFixedDamageTypeTag();
	static FName GetDamageAttributeCoefficientName(EMADamageAttributeSide Side, const FGameplayAttribute& Attribute);
	static void ApplyDamageExecutionConfig(FGameplayEffectSpecHandle& SpecHandle, const FMADamageExecutionConfig& DamageConfig);
	static void SetReactionSourcePoint(FGameplayEffectSpecHandle& SpecHandle, const FVector& SourcePoint);
	static bool TryGetReactionSourcePoint(const FGameplayEffectSpec& Spec, FVector& OutSourcePoint);

	static FGameplayTag GetAnyReactionStateTag();
	static bool IsPlayer(const AActor* ActorToCheck);

	static bool CheckAbilityCost(const FGameplayAbilitySpec& AbilitySpec, const UAbilitySystemComponent& ASC);
	static bool CheckAbilityCostStatic(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC);
	static float GetCooldownDurationFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC, int AbilityLevel);
	static float GetCooldownRemainingFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC);

};
