// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MAAbilitySystemStatics.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"
#include "MAAbilitySystemComponent.h"
#include "MAGameplayAbilityTypes.h"
#include "Ability/MAGameplayAbility_Skill.h"
#include "Setting/MASkillSubsystem.h"

FGameplayTag UMAAbilitySystemStatics::GetBasicAttackAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Attack.Basic");
}

FGameplayTag UMAAbilitySystemStatics::GetBasicAttackInputPressedTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Attack.Basic.Pressed");
}

FGameplayTag UMAAbilitySystemStatics::GetBasicAttackInputReleasedTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Attack.Basic.Released");
}

FGameplayTag UMAAbilitySystemStatics::GetSkillAttackTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Attack.Skill");
}

FGameplayTag UMAAbilitySystemStatics::GetIgnoreClearTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Combo.Clear");
}

FGameplayTag UMAAbilitySystemStatics::GetDeadStatTag()
{
	return FGameplayTag::RequestGameplayTag("State.Dead");
}

FGameplayTag UMAAbilitySystemStatics::GetRotationLockTag()
{
	return FGameplayTag::RequestGameplayTag("State.RotationLock");
}

FGameplayTag UMAAbilitySystemStatics::GetRushingTag()
{
	return FGameplayTag::RequestGameplayTag("State.Rushing");
}

FGameplayTag UMAAbilitySystemStatics::GetAimingTag()
{
	return FGameplayTag::RequestGameplayTag("State.Aiming");
}

FGameplayTag UMAAbilitySystemStatics::GetMoveBlockTag()
{
	return FGameplayTag::RequestGameplayTag("State.MoveBlocked");
}

FGameplayTag UMAAbilitySystemStatics::GetHealthFullStatTag()
{
	return FGameplayTag::RequestGameplayTag("Stats.Health.Full");
}

FGameplayTag UMAAbilitySystemStatics::GetHealthEmptyStatTag()
{
	return FGameplayTag::RequestGameplayTag("Stats.Health.Empty");
}

FGameplayTag UMAAbilitySystemStatics::GetPlayerRoleTag()
{
	return FGameplayTag::RequestGameplayTag("role.Player");
}

FGameplayTag UMAAbilitySystemStatics::GetGoldAttributeTag()
{
	return FGameplayTag::RequestGameplayTag("attr.gold");
}

FGameplayTag UMAAbilitySystemStatics::GetMeleeActionTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Action.Melee");
}

FGameplayTag UMAAbilitySystemStatics::GetProjectileActionTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Action.Projectile");
}

FGameplayTag UMAAbilitySystemStatics::GetTargetingActionTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Action.Targeting");
}

FGameplayTag UMAAbilitySystemStatics::GetMontageDamageTag()
{
	return FGameplayTag::RequestGameplayTag("Event.Montage.Damage");
}

FGameplayTag UMAAbilitySystemStatics::GetMontageProjectileTag()
{
	return FGameplayTag::RequestGameplayTag("Event.Montage.SpawnProjectile");
}

FGameplayTag UMAAbilitySystemStatics::GetLaunchActivateTag()
{
	return FGameplayTag::RequestGameplayTag("Event.Montage.LaunchActivate");
}

FGameplayTag UMAAbilitySystemStatics::GetBehaviorMultiplierTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Damage.BehaviorModifier");
}

FGameplayTag UMAAbilitySystemStatics::GetElementalMultiplierTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Damage.ElementalModifier");
}

FGameplayTag UMAAbilitySystemStatics::GetUtilityMultiplierTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Damage.UtilityModifier");
}

FGameplayTag UMAAbilitySystemStatics::GetDamageBaseTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Damage.Base");
}

FGameplayTag UMAAbilitySystemStatics::GetDamageAttributeCoefficientTag(EMADamageAttributeSide Side, EMADamageAttribute Attribute)
{
	const TCHAR* SideName = Side == EMADamageAttributeSide::Source ? TEXT("Source") : TEXT("Target");
	const TCHAR* AttributeName = TEXT("Attack");

	switch (Attribute)
	{
	case EMADamageAttribute::Health: AttributeName = TEXT("Health"); break;
	case EMADamageAttribute::MaxHealth: AttributeName = TEXT("MaxHealth"); break;
	case EMADamageAttribute::Attack: AttributeName = TEXT("Attack"); break;
	case EMADamageAttribute::MoveSpeed: AttributeName = TEXT("MoveSpeed"); break;
	case EMADamageAttribute::AttackSpeed: AttributeName = TEXT("AttackSpeed"); break;
	case EMADamageAttribute::Armor: AttributeName = TEXT("Armor"); break;
	case EMADamageAttribute::ArmorPenetration: AttributeName = TEXT("ArmorPenetration"); break;
	case EMADamageAttribute::CriticalChance: AttributeName = TEXT("CriticalChance"); break;
	case EMADamageAttribute::CriticalDamage: AttributeName = TEXT("CriticalDamage"); break;
	}

	return FGameplayTag::RequestGameplayTag(*FString::Printf(TEXT("Data.Damage.Coeff.%s.%s"), SideName, AttributeName));
}

void UMAAbilitySystemStatics::ApplyDamageExecutionConfig(FGameplayEffectSpecHandle& SpecHandle, const FMADamageExecutionConfig& DamageConfig)
{
	if (!SpecHandle.IsValid()) return;

	TMap<FGameplayTag, float> SummedMagnitudes;
	SummedMagnitudes.FindOrAdd(GetDamageBaseTag()) += DamageConfig.BaseDamage;

	for (const FMADamageAttributeCoefficient& Coefficient : DamageConfig.AttributeCoefficients)
	{
		if (FMath::IsNearlyZero(Coefficient.Coefficient)) continue;

		SummedMagnitudes.FindOrAdd(GetDamageAttributeCoefficientTag(Coefficient.Side, Coefficient.Attribute)) += Coefficient.Coefficient;
	}

	for (const TPair<FGameplayTag, float>& Pair : SummedMagnitudes)
	{
		if (FMath::IsNearlyZero(Pair.Value)) continue;

		SpecHandle.Data->SetSetByCallerMagnitude(Pair.Key, Pair.Value);
	}
}

FGameplayTag UMAAbilitySystemStatics::GetStunStatTag()
{
	return FGameplayTag::RequestGameplayTag("State.Debuff.Stun");
}

FGameplayTag UMAAbilitySystemStatics::GetHitReactTag()
{
	return FGameplayTag::RequestGameplayTag("State.Debuff.HitReact");
}

FGameplayTag UMAAbilitySystemStatics::GetAnyReactionStateTag()
{
	return FGameplayTag::RequestGameplayTag("State.Debuff");
}

FGameplayTag UMAAbilitySystemStatics::GetAirborneTag()
{
	return FGameplayTag::RequestGameplayTag("State.Debuff.Airborne");
}

FGameplayTag UMAAbilitySystemStatics::GetKnockdownTag()
{
	return FGameplayTag::RequestGameplayTag("State.Debuff.Knockdown");
}

FGameplayTag UMAAbilitySystemStatics::GetRecoveryTag()
{
	return FGameplayTag::RequestGameplayTag("Stats.Recovery");
}

bool UMAAbilitySystemStatics::IsPlayer(const AActor* ActorToCheck)
{
	const IAbilitySystemInterface* ActorISA = Cast<IAbilitySystemInterface>(ActorToCheck);
	if (ActorISA)
	{
		UAbilitySystemComponent* ActorASC = ActorISA->GetAbilitySystemComponent();
		if (ActorASC)
		{
			return ActorASC->HasMatchingGameplayTag(GetPlayerRoleTag());
		}
	}
	return false;
}

float UMAAbilitySystemStatics::GetStaticCooldownDurationForAbility(const UGameplayAbility* Ability)
{
	if (!Ability)
		return 0.f;
	
	const UGameplayEffect* CooldownEffect = Ability->GetCooldownGameplayEffect();
	if (!CooldownEffect)
		return 0.f;

	float CooldownDuration = 0.f;

	CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(1, CooldownDuration);
	return CooldownDuration;
}

float UMAAbilitySystemStatics::GetStaticCostForAbility(const UGameplayAbility* Ability)
{
	if (!Ability)
		return 0.f;

	const UGameplayEffect* CostEffect = Ability->GetCostGameplayEffect();
	if (!CostEffect || CostEffect->Modifiers.Num() == 0)
		return 0.f;

	float Cost = 0.f;
	CostEffect->Modifiers[0].ModifierMagnitude.GetStaticMagnitudeIfPossible(1, Cost);
	return FMath::Abs(Cost);
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

float UMAAbilitySystemStatics::GetExpectedCooldownDuration(const UGameplayAbility* AbilityCDO,	const UAbilitySystemComponent* ASC)
{
	if (!AbilityCDO || !ASC)
		return 0.f;

	const UMAGameplayAbility_Skill* SkillAbility = Cast<UMAGameplayAbility_Skill>(AbilityCDO);
	if (!SkillAbility)
		return 0.f;

	float FinalCooldown = 0.f;
	const FSkillData* FetchedSkillData = nullptr;
	
	if (UWorld* World = ASC->GetWorld())
	{
		if (UMASkillSubsystem* SkillSys = World->GetGameInstance()->GetSubsystem<UMASkillSubsystem>())
		{
			FetchedSkillData = SkillSys->GetSkillData(SkillAbility->GetSkillID());
			if (FetchedSkillData)
			{
				FinalCooldown = FetchedSkillData->BaseCooldown;

				if (const FModuleUtilityData* UtilityRow = SkillSys->GetUtilityData(FetchedSkillData->DefaultUtilityTag))
				{
					FinalCooldown *= UtilityRow->CooldownMultiplier;
				}
			}
		}
	}
	return FinalCooldown;
}
