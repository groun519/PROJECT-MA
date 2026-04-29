#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MAAbilitySystemStatics.generated.h"


class UGameplayAbility;
struct FGameplayAbilitySpec;
class UAbilitySystemComponent;
enum class EMADamageAttribute : uint8;
enum class EMADamageAttributeSide : uint8;
struct FMADamageExecutionConfig;
/**
 * 
 */
UCLASS()
class UMAAbilitySystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static FGameplayTag GetSkillAttackTag();
	static FGameplayTag GetIgnoreClearTag();
	static FGameplayTag GetDeadStatTag();
	static FGameplayTag GetStunStatTag();
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

	static FGameplayTag GetGoldAttributeTag();

	static FGameplayTag GetMeleeActionTag();
	static FGameplayTag GetProjectileActionTag();
	static FGameplayTag GetTargetingActionTag();
	
	static FGameplayTag GetMontageDamageTag();
	static FGameplayTag GetMontageProjectileTag();

	static FGameplayTag GetBehaviorMultiplierTag();
	static FGameplayTag GetElementalMultiplierTag();
	static FGameplayTag GetUtilityMultiplierTag();
	static FGameplayTag GetDamageBaseTag();
	static FGameplayTag GetFinalDamageMultiplierTag();
	static FGameplayTag GetDamageAttributeCoefficientTag(EMADamageAttributeSide Side, EMADamageAttribute Attribute);
	static void ApplyDamageExecutionConfig(FGameplayEffectSpecHandle& SpecHandle, const FMADamageExecutionConfig& DamageConfig);
	static void SetReactionSourcePoint(FGameplayEffectSpecHandle& SpecHandle, const FVector& SourcePoint);
	static bool TryGetReactionSourcePoint(const FGameplayEffectSpec& Spec, FVector& OutSourcePoint);

	static FGameplayTag GetAnyReactionStateTag();
	static bool IsPlayer(const AActor* ActorToCheck);

	static float GetStaticCooldownDurationForAbility(const UGameplayAbility* Ability);
	static float GetStaticCostForAbility(const UGameplayAbility* Ability);

	static bool CheckAbilityCost(const FGameplayAbilitySpec& AbilitySpec, const UAbilitySystemComponent& ASC);
	static bool CheckAbilityCostStatic(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC);
	static float GetCooldownDurationFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC, int AbilityLevel);
	static float GetCooldownRemainingFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC);

	static float GetExpectedCooldownDuration(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent* ASC);
};
