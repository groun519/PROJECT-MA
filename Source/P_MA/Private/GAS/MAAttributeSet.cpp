#include "GAS/MAAttributeSet.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "MAGameplayAbilityTypes.h"

/*
* void UNVAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
* {
* 	GAMEPLAYATTRIBUTE_REPNOTIFY(UNVAttributeSet, Health, OldValue)
* }
*
* ^ ^ ^ ^ ^ ^ ^ ^ ^ ^
*
* DEFINE_REPNOTIFY(Health)
*/
#define DEFINE_REPNOTIFY(PropertyName)                              \
void UMAAttributeSet::OnRep_##PropertyName(                     \
const FGameplayAttributeData& OldValue)                     \
{                                                               \
GAMEPLAYATTRIBUTE_REPNOTIFY(UMAAttributeSet, PropertyName, OldValue); \
}

void UMAAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Attack, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, DamageVariance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Fury, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, MaxFury, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, CriticalChance, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, CriticalDamage, COND_None, REPNOTIFY_Always)
}

void UMAAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
}

void UMAAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	float DeltaHealth = 0.f;
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		const float RawHealth = GetHealth();
		const float ClampedHealth = FMath::Clamp(RawHealth, 0.f, GetMaxHealth());
		DeltaHealth = ClampedHealth - (RawHealth - Data.EvaluatedData.Magnitude);
		SetHealth(ClampedHealth);
	}
	else
	{
		return;
	}

	if (FMath::IsNearlyZero(DeltaHealth)) return;

	FGameplayEffectContextHandle ContextHandle = Data.EffectSpec.GetContext();

	if (DeltaHealth < 0.f)
	{
		float FinalDamage = -DeltaHealth;

		const FMAGameplayEffectContext* MAContext = static_cast<const FMAGameplayEffectContext*>(ContextHandle.Get());
		bool bIsCriticalHit = false;
		if (MAContext)
		{
			bIsCriticalHit = MAContext->IsCriticalHit();
		}
		AActor* TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();

		FMADamageAppliedEvent DamageEvent;
		DamageEvent.SourceActor = ContextHandle.GetOriginalInstigator();
		DamageEvent.TargetActor = TargetActor;
		DamageEvent.Amount = FinalDamage;
		DamageEvent.DamageTypeTag = MAContext && MAContext->GetDamageTypeTag().IsValid()
			? MAContext->GetDamageTypeTag()
			: UMAAbilitySystemStatics::GetDefaultDamageTypeTag();
		DamageEvent.bIsCriticalHit = bIsCriticalHit;
		if (const FHitResult* HitResult = ContextHandle.GetHitResult())
		{
			DamageEvent.HitResult = *HitResult;
		}

		UMAAbilitySystemComponent* SourceASC = Cast<UMAAbilitySystemComponent>(ContextHandle.GetOriginalInstigatorAbilitySystemComponent());
		UMAAbilitySystemComponent* TargetASC = Cast<UMAAbilitySystemComponent>(&Data.Target);
		if (SourceASC && SourceASC != TargetASC)
		{
			SourceASC->NotifyDamageApplied(DamageEvent, false);
		}
		if (TargetASC)
		{
			TargetASC->NotifyDamageApplied(DamageEvent, true);
		}

		if (MAContext)
		{
			UMASkillAbility* SkillAbility = MAContext->GetSkillEventAbility();
			UMASkillModuleInstance* SkillEventScope = MAContext->GetSkillEventScope();
			if (SkillAbility && SkillEventScope)
			{
				FGameplayEventData EventData;
				EventData.Instigator = DamageEvent.SourceActor.Get();
				EventData.Target = DamageEvent.TargetActor.Get();
				EventData.EventMagnitude = DamageEvent.Amount;
				if (const FHitResult* HitResult = ContextHandle.GetHitResult())
				{
					EventData.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(*HitResult);
				}
				SkillEventScope->BroadcastScopedEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Skill.DamageDealt")), EventData);
			}
		}
	}
	else if (DeltaHealth > 0.f)
	{
		const FMAGameplayEffectContext* MAContext = static_cast<const FMAGameplayEffectContext*>(ContextHandle.Get());
		if (!MAContext || !MAContext->GetDamageTypeTag().MatchesTag(UMAAbilitySystemStatics::GetHealDamageTypeTag())) return;

		AActor* TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();

		FMADamageAppliedEvent DamageEvent;
		DamageEvent.SourceActor = ContextHandle.GetOriginalInstigator();
		DamageEvent.TargetActor = TargetActor;
		DamageEvent.Amount = DeltaHealth;
		DamageEvent.DamageTypeTag = MAContext->GetDamageTypeTag();
		if (const FHitResult* HitResult = ContextHandle.GetHitResult())
		{
			DamageEvent.HitResult = *HitResult;
		}

		UMAAbilitySystemComponent* SourceASC = Cast<UMAAbilitySystemComponent>(ContextHandle.GetOriginalInstigatorAbilitySystemComponent());
		UMAAbilitySystemComponent* TargetASC = Cast<UMAAbilitySystemComponent>(&Data.Target);
		if (SourceASC && SourceASC != TargetASC)
		{
			SourceASC->NotifyDamageApplied(DamageEvent, false);
		}
		if (TargetASC)
		{
			TargetASC->NotifyDamageApplied(DamageEvent, true);
		}
	}
}

DEFINE_REPNOTIFY(Health)
DEFINE_REPNOTIFY(MaxHealth)
DEFINE_REPNOTIFY(Attack)
DEFINE_REPNOTIFY(DamageVariance)
DEFINE_REPNOTIFY(MoveSpeed)
DEFINE_REPNOTIFY(AttackSpeed)
DEFINE_REPNOTIFY(Armor)
DEFINE_REPNOTIFY(ArmorPenetration)
DEFINE_REPNOTIFY(Fury)
DEFINE_REPNOTIFY(MaxFury)
DEFINE_REPNOTIFY(CriticalChance)
DEFINE_REPNOTIFY(CriticalDamage)
