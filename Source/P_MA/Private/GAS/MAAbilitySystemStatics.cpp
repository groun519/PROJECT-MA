#include "GAS/MAAbilitySystemStatics.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "MAGameplayAbilityTypes.h"
#include "Player/MAPlayerCharacter.h"

FGameplayTag UMAAbilitySystemStatics::GetDeadStatTag()
{
	return FGameplayTag::RequestGameplayTag("State.Dead");
}

FGameplayTag UMAAbilitySystemStatics::GetRotationLockTag()
{
	return FGameplayTag::RequestGameplayTag("State.RotationLock");
}

FGameplayTag UMAAbilitySystemStatics::GetInputBlockTag()
{
	return FGameplayTag::RequestGameplayTag("State.InputBlocked");
}

FGameplayTag UMAAbilitySystemStatics::GetMoveBlockTag()
{
	return FGameplayTag::RequestGameplayTag("State.MoveBlocked");
}

FGameplayTag UMAAbilitySystemStatics::GetAbilityBlockTag()
{
	return FGameplayTag::RequestGameplayTag("State.AbilityBlocked");
}

FGameplayTag UMAAbilitySystemStatics::GetReactionSourceXTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Reaction.Source.X");
}

FGameplayTag UMAAbilitySystemStatics::GetReactionSourceYTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Reaction.Source.Y");
}

FGameplayTag UMAAbilitySystemStatics::GetReactionSourceZTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Reaction.Source.Z");
}

FGameplayTag UMAAbilitySystemStatics::GetHealthFullStatTag()
{
	return FGameplayTag::RequestGameplayTag("Stats.Health.Full");
}

FGameplayTag UMAAbilitySystemStatics::GetHealthEmptyStatTag()
{
	return FGameplayTag::RequestGameplayTag("Stats.Health.Empty");
}

FGameplayTag UMAAbilitySystemStatics::GetDefaultVisualElementTag()
{
	return FGameplayTag::RequestGameplayTag("Module.Visual.Elemental.Default");
}

FGameplayTag UMAAbilitySystemStatics::GetPlayerRespawnGameplayCueTag()
{
	return FGameplayTag::RequestGameplayTag("GameplayCue.Player.Respawn");
}

FGameplayTag UMAAbilitySystemStatics::GetBehaviorMultiplierTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Damage.BehaviorModifier");
}

FGameplayTag UMAAbilitySystemStatics::GetDamageBaseTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Damage.Base");
}

FGameplayTag UMAAbilitySystemStatics::GetAppliedDamageTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Damage.Applied");
}

FGameplayTag UMAAbilitySystemStatics::GetDamageTargetTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Damage.Target");
}

FGameplayTag UMAAbilitySystemStatics::GetFinalDamageMultiplierTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Damage.FinalModifier");
}

FGameplayTag UMAAbilitySystemStatics::GetDamageVarianceTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Damage.Variance");
}

FGameplayTag UMAAbilitySystemStatics::GetModuleStackTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Module.Stack");
}

FGameplayTag UMAAbilitySystemStatics::GetModuleLinkedGameplayEffectHandleTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Module.LinkedGameplayEffectHandle");
}

FGameplayTag UMAAbilitySystemStatics::GetSkillAttackSpeedMultiplierTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Skill.AttackSpeedMultiplier");
}

FGameplayTag UMAAbilitySystemStatics::GetSkillFocusOffsetTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Skill.FocusOffset");
}

FGameplayTag UMAAbilitySystemStatics::GetSkillAreaScaleTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Skill.AreaScale");
}

FGameplayTag UMAAbilitySystemStatics::GetSkillChargeRatioTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Skill.Payload.Scalar.ChargeRatio");
}

FGameplayTag UMAAbilitySystemStatics::GetHitEventTag()
{
	return FGameplayTag::RequestGameplayTag("Event.Skill.Hit");
}

FGameplayTag UMAAbilitySystemStatics::GetMovementStartEventTag()
{
	return FGameplayTag::RequestGameplayTag("Event.Skill.MovementStart");
}

FGameplayTag UMAAbilitySystemStatics::GetChargeCompletedEventTag()
{
	return FGameplayTag::RequestGameplayTag("Event.Skill.ChargeCompleted");
}

FGameplayTag UMAAbilitySystemStatics::GetModuleActivationChangedEventTag()
{
	return FGameplayTag::RequestGameplayTag("Event.Module.ActivationChanged");
}

FGameplayTag UMAAbilitySystemStatics::GetModuleStackChangedEventTag()
{
	return FGameplayTag::RequestGameplayTag("Event.Module.StackChanged");
}

FGameplayTag UMAAbilitySystemStatics::GetMovementHandleTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Event.MovementHandle");
}

FGameplayTag UMAAbilitySystemStatics::GetDefaultDamageTypeTag()
{
	return FGameplayTag::RequestGameplayTag("DamageType.Damage");
}

FGameplayTag UMAAbilitySystemStatics::GetHealDamageTypeTag()
{
	return FGameplayTag::RequestGameplayTag("DamageType.Heal");
}

FGameplayTag UMAAbilitySystemStatics::GetFireDamageTypeTag()
{
	return FGameplayTag::RequestGameplayTag("DamageType.Fire");
}

FGameplayTag UMAAbilitySystemStatics::GetIceDamageTypeTag()
{
	return FGameplayTag::RequestGameplayTag("DamageType.Ice");
}

FGameplayTag UMAAbilitySystemStatics::GetFixedDamageTypeTag()
{
	return FGameplayTag::RequestGameplayTag("DamageType.Fixed");
}

FName UMAAbilitySystemStatics::GetDamageAttributeCoefficientName(
	EMADamageAttributeSide Side,
	const FGameplayAttribute& Attribute)
{
	if (!Attribute.IsValid() || Side == EMADamageAttributeSide::Payload) return NAME_None;

	const TCHAR* SideName = Side == EMADamageAttributeSide::Source ? TEXT("Source") : TEXT("Target");
	return FName(*FString::Printf(
		TEXT("Data.Damage.Coeff.%s.%s.%s"),
		SideName,
		*GetNameSafe(Attribute.GetAttributeSetClass()),
		*Attribute.GetName()));
}

void UMAAbilitySystemStatics::ApplyDamageExecutionConfig(FGameplayEffectSpecHandle& SpecHandle, const FMADamageExecutionConfig& DamageConfig)
{
	if (!SpecHandle.IsValid()) return;

	TMap<FName, float> SummedAttributeCoefficients;
	SpecHandle.Data->SetSetByCallerMagnitude(GetDamageBaseTag(), DamageConfig.BaseDamage);

	for (const FMADamageAttributeCoefficient& Coefficient : DamageConfig.AttributeCoefficients)
	{
		if (FMath::IsNearlyZero(Coefficient.Coefficient)) continue;
		if (Coefficient.Side == EMADamageAttributeSide::Payload) continue;

		const FName CoefficientName = GetDamageAttributeCoefficientName(
			Coefficient.Side,
			Coefficient.GameplayAttribute);
		if (!CoefficientName.IsNone())
		{
			SummedAttributeCoefficients.FindOrAdd(CoefficientName) += Coefficient.Coefficient;
		}
	}

	for (const TPair<FName, float>& Pair : SummedAttributeCoefficients)
	{
		if (FMath::IsNearlyZero(Pair.Value)) continue;

		SpecHandle.Data->SetSetByCallerMagnitude(Pair.Key, Pair.Value);
	}

	FGameplayEffectContextHandle ContextHandle = SpecHandle.Data->GetContext();
	if (FMAGameplayEffectContext* MAContext = static_cast<FMAGameplayEffectContext*>(ContextHandle.Get()))
	{
		MAContext->SetDamageTypeTag(DamageConfig.DamageTypeTag);
	}
}

void UMAAbilitySystemStatics::SetReactionSourcePoint(FGameplayEffectSpecHandle& SpecHandle, const FVector& SourcePoint)
{
	if (!SpecHandle.IsValid()) return;

	// TODO: If reaction payload grows beyond SourcePoint, move this data to a custom GameplayEffectContext.
	SpecHandle.Data->SetSetByCallerMagnitude(GetReactionSourceXTag(), SourcePoint.X);
	SpecHandle.Data->SetSetByCallerMagnitude(GetReactionSourceYTag(), SourcePoint.Y);
	SpecHandle.Data->SetSetByCallerMagnitude(GetReactionSourceZTag(), SourcePoint.Z);
}

bool UMAAbilitySystemStatics::TryGetReactionSourcePoint(const FGameplayEffectSpec& Spec, FVector& OutSourcePoint)
{
	// TODO: If reaction payload grows beyond SourcePoint, read it from a custom GameplayEffectContext instead.
	const float* SourceX = Spec.SetByCallerTagMagnitudes.Find(GetReactionSourceXTag());
	const float* SourceY = Spec.SetByCallerTagMagnitudes.Find(GetReactionSourceYTag());
	const float* SourceZ = Spec.SetByCallerTagMagnitudes.Find(GetReactionSourceZTag());
	if (!SourceX || !SourceY || !SourceZ) return false;

	OutSourcePoint.X = *SourceX;
	OutSourcePoint.Y = *SourceY;
	OutSourcePoint.Z = *SourceZ;
	return true;
}

FGameplayTag UMAAbilitySystemStatics::GetStunStatTag()
{
	return FGameplayTag::RequestGameplayTag("State.Debuff.Stun");
}

FGameplayTag UMAAbilitySystemStatics::GetFrozenStatTag()
{
	return FGameplayTag::RequestGameplayTag("State.Debuff.Frozen");
}

FGameplayTag UMAAbilitySystemStatics::GetRootStatTag()
{
	return FGameplayTag::RequestGameplayTag("State.Debuff.Root");
}

FGameplayTag UMAAbilitySystemStatics::GetAirborneStatTag()
{
	return FGameplayTag::RequestGameplayTag("State.Debuff.Airborne");
}

FGameplayTag UMAAbilitySystemStatics::GetAirborneRiseTimeTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Reaction.Airborne.RiseTime");
}

FGameplayTag UMAAbilitySystemStatics::GetGrabStatTag()
{
	return FGameplayTag::RequestGameplayTag("State.Debuff.Grab");
}

FGameplayTag UMAAbilitySystemStatics::GetStaggerStatTag()
{
	return FGameplayTag::RequestGameplayTag("State.Debuff.Stagger");
}

FGameplayTag UMAAbilitySystemStatics::GetKnockbackStatTag()
{
	return FGameplayTag::RequestGameplayTag("State.Debuff.Knockback");
}

FGameplayTag UMAAbilitySystemStatics::GetAnyReactionStateTag()
{
	return FGameplayTag::RequestGameplayTag("State.Debuff");
}

bool UMAAbilitySystemStatics::IsPlayer(const AActor* ActorToCheck)
{
	return ActorToCheck && ActorToCheck->IsA<AMAPlayerCharacter>();
}

bool UMAAbilitySystemStatics::CheckAbilityCost(const FGameplayAbilitySpec& AbilitySpec, const UAbilitySystemComponent& ASC)
{
	const UGameplayAbility* AbilityCDO = AbilitySpec.Ability;
	if (AbilityCDO)
	{
		return AbilityCDO->CheckCost(AbilitySpec.Handle, ASC.AbilityActorInfo.Get());
	}

	return false;
}

bool UMAAbilitySystemStatics::CheckAbilityCostStatic(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC)
{
	if (AbilityCDO)
	{
		return AbilityCDO->CheckCost(FGameplayAbilitySpecHandle(), ASC.AbilityActorInfo.Get());
	}

	return false;
}

float UMAAbilitySystemStatics::GetCooldownDurationFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC, int AbilityLevel)
{
	float CooldownDuration = 0.f;
	if (AbilityCDO)
	{
		UGameplayEffect* CooldownEffect = AbilityCDO->GetCooldownGameplayEffect();
		if (CooldownEffect)
		{
			FGameplayEffectSpecHandle EffectSpec = ASC.MakeOutgoingSpec(CooldownEffect->GetClass(), AbilityLevel, ASC.MakeEffectContext());
			CooldownEffect->DurationMagnitude.AttemptCalculateMagnitude(*EffectSpec.Data.Get(), CooldownDuration);
		}
	}

	return FMath::Abs(CooldownDuration);
}

float UMAAbilitySystemStatics::GetCooldownRemainingFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC)
{
	if (!AbilityCDO)
		return 0;

	UGameplayEffect* CooldownEffect = AbilityCDO->GetCooldownGameplayEffect();
	if (!CooldownEffect)
		return 0;

	FGameplayEffectQuery CooldownEffectQuery;
	CooldownEffectQuery.EffectDefinition = CooldownEffect->GetClass();

	float CooldownRemaining = 0.f;
	FJsonSerializableArrayFloat CooldownTimeRemainings = ASC.GetActiveEffectsTimeRemaining(CooldownEffectQuery);

	for (float Remaining : CooldownTimeRemainings)
	{
		if (Remaining > CooldownRemaining)
		{
			CooldownRemaining = Remaining;
		}
	}

	return CooldownRemaining;
}
