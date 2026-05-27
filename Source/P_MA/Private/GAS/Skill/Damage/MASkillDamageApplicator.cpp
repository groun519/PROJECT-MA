#include "GAS/Skill/Damage/MASkillDamageApplicator.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Engine/HitResult.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GameplayEffect.h"

MASkillDamageApplicator::FMASkillDamageApplicationContext MASkillDamageApplicator::MakeApplicationContext(
	const UMASkillAbility& OwnerAbility,
	UMASkillModuleInstance* SkillEventScope,
	const FVector& StatusEffectSourcePoint)
{
	FMASkillDamageApplicationContext Context;
	Context.InstigatorActor = OwnerAbility.GetAvatarActorFromActorInfo();
	Context.EffectCauser = Context.InstigatorActor;
	Context.SkillAbility = const_cast<UMASkillAbility*>(&OwnerAbility);
	Context.SkillEventScope = SkillEventScope;
	Context.StatusEffectSourcePoint = StatusEffectSourcePoint;
	return Context;
}

FVector MASkillDamageApplicator::ResolveStatusEffectSourcePoint(
	const FMASkillDamageApplicationContext& ApplicationContext,
	EMASkillStatusEffectSourceType SourceType)
{
	switch (SourceType)
	{
	case EMASkillStatusEffectSourceType::Center:
		return ApplicationContext.StatusEffectSourcePoint;
	case EMASkillStatusEffectSourceType::Instigator:
	default:
		if (const AActor* InstigatorActor = ApplicationContext.InstigatorActor)
		{
			return InstigatorActor->GetActorLocation();
		}
		return ApplicationContext.StatusEffectSourcePoint;
	}
}

bool MASkillDamageApplicator::ShouldApplyResolvedStatusEffect(
	UAbilitySystemComponent& TargetASC,
	AActor* TargetActor,
	const FResolvedStatusEffect& StatusEffect)
{
	if (StatusEffect.StrengthPolicy == EMASkillStatusEffectStrengthPolicy::None) return true;
	if (!TargetActor || !StatusEffect.SpecHandle.IsValid() || !StatusEffect.SpecHandle.Data.IsValid()) return true;

	const UGameplayEffect* EffectDefinition = StatusEffect.SpecHandle.Data->Def.Get();
	if (!EffectDefinition) return true;

	const UClass* EffectDefinitionClass = EffectDefinition->GetClass();
	FGameplayEffectQuery Query(FActiveGameplayEffectQueryCustomMatch::CreateLambda(
		[EffectDefinitionClass](const FActiveGameplayEffect& ActiveEffect)
		{
			const UGameplayEffect* ActiveDefinition = ActiveEffect.Spec.Def.Get();
			return ActiveDefinition && ActiveDefinition->GetClass() == EffectDefinitionClass;
		}));

	const TArray<FActiveGameplayEffectHandle> ActiveEffects = TargetASC.GetActiveEffects(Query);
	if (ActiveEffects.Num() == 0) return true;

	bool bApplyIncomingEffect = true;
	for (const FActiveGameplayEffectHandle ActiveEffectHandle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveEffect = TargetASC.GetActiveGameplayEffect(ActiveEffectHandle);
		if (!ActiveEffect) continue;

		const float ExistingStrengthMagnitude = ActiveEffect->Spec.GetSetByCallerMagnitude(
			FGameplayTag::RequestGameplayTag(TEXT("Data.StatusEffect.StrengthMagnitude")),
			false,
			StatusEffect.StrengthMagnitude);

		switch (StatusEffect.StrengthPolicy)
		{
		case EMASkillStatusEffectStrengthPolicy::LargerMagnitudeStronger:
			if (ExistingStrengthMagnitude > StatusEffect.StrengthMagnitude
				&& !FMath::IsNearlyEqual(ExistingStrengthMagnitude, StatusEffect.StrengthMagnitude))
			{
				bApplyIncomingEffect = false;
			}
			break;
		case EMASkillStatusEffectStrengthPolicy::SmallerMagnitudeStronger:
			if (ExistingStrengthMagnitude < StatusEffect.StrengthMagnitude
				&& !FMath::IsNearlyEqual(ExistingStrengthMagnitude, StatusEffect.StrengthMagnitude))
			{
				bApplyIncomingEffect = false;
			}
			break;
		default:
			break;
		}

		if (!bApplyIncomingEffect) return false;
	}

	for (const FActiveGameplayEffectHandle ActiveEffectHandle : ActiveEffects)
	{
		TargetASC.RemoveActiveGameplayEffect(ActiveEffectHandle);
	}

	return true;
}

void MASkillDamageApplicator::ApplySpecToTargetASC(
	UAbilitySystemComponent& TargetASC,
	const FHitResult& HitResult,
	const FMASkillDamageApplicationContext& ApplicationContext,
	const FGameplayEffectSpecHandle& SpecHandle)
{
	if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
	{
		return;
	}

	FGameplayEffectSpecHandle LocalSpecHandle;
	LocalSpecHandle.Data = MakeShared<FGameplayEffectSpec>(*SpecHandle.Data.Get());

	FGameplayEffectContextHandle ContextHandle = LocalSpecHandle.Data->GetContext().Duplicate();
	ContextHandle.AddHitResult(HitResult, true);
	if (FMAGameplayEffectContext* MAContext = static_cast<FMAGameplayEffectContext*>(ContextHandle.Get()))
	{
		MAContext->SetSkillEventContext(ApplicationContext.SkillAbility, ApplicationContext.SkillEventScope);
	}
	LocalSpecHandle.Data->SetContext(ContextHandle);

	TargetASC.ApplyGameplayEffectSpecToSelf(*LocalSpecHandle.Data.Get());
}

static void NotifySkillHit(
	const MASkillDamageApplicator::FMASkillDamageApplicationContext& ApplicationContext,
	const FHitResult& HitResult)
{
	if (!ApplicationContext.SkillAbility || !ApplicationContext.SkillEventScope) return;

	FGameplayEventData EventData;
	EventData.Instigator = ApplicationContext.InstigatorActor;
	EventData.Target = HitResult.GetActor();
	EventData.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(HitResult);
	ApplicationContext.SkillEventScope->BroadcastScopedEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Skill.Hit")), EventData);
}

void MASkillDamageApplicator::ExecuteTargetGameplayCues(
	UAbilitySystemComponent& TargetASC,
	const FHitResult& HitResult,
	const FResolvedSkillHitEffects& ResolvedHitEffects,
	const FMASkillDamageApplicationContext& ApplicationContext)
{
	if (ResolvedHitEffects.TargetGameplayCueTags.IsEmpty()) return;

	FGameplayCueParameters CueParams;
	CueParams.Location = HitResult.ImpactPoint.IsNearlyZero() ? HitResult.Location : HitResult.ImpactPoint;
	CueParams.Normal = HitResult.ImpactNormal.IsNearlyZero() ? HitResult.Normal : HitResult.ImpactNormal;
	CueParams.Instigator = ApplicationContext.InstigatorActor;
	CueParams.EffectCauser = ApplicationContext.EffectCauser ? ApplicationContext.EffectCauser : ApplicationContext.InstigatorActor;

	for (const FGameplayTag& GameplayCueTag : ResolvedHitEffects.TargetGameplayCueTags)
	{
		if (!GameplayCueTag.IsValid()) continue;

		FGameplayCueParameters CueParamsForTag = CueParams;
		CueParamsForTag.OriginalTag = GameplayCueTag;
		CueParamsForTag.AggregatedSourceTags.AddTag(GameplayCueTag);
		TargetASC.ExecuteGameplayCue(GameplayCueTag, CueParamsForTag);
	}
}

void MASkillDamageApplicator::ApplyHitResults(
	UMASkillAbility& OwnerAbility,
	UMASkillModuleInstance* SkillEventScope,
	const TArray<FHitResult>& HitResults,
	const FResolvedSkillHitEffects& ResolvedHitEffects,
	const FVector& StatusEffectSourcePoint)
{
	TSet<AActor*> HitActors;
	const FMASkillDamageApplicationContext ApplicationContext = MakeApplicationContext(OwnerAbility, SkillEventScope, StatusEffectSourcePoint);

	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor || HitActors.Contains(HitActor))
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!TargetASC)
		{
			continue;
		}

		ApplyToTarget(*TargetASC, HitResult, ResolvedHitEffects, ApplicationContext);
		HitActors.Add(HitActor);
	}
}

void MASkillDamageApplicator::ApplyHitResult(
	UMASkillAbility& OwnerAbility,
	UMASkillModuleInstance* SkillEventScope,
	const FHitResult& HitResult,
	const FResolvedSkillHitEffects& ResolvedHitEffects,
	const FVector& StatusEffectSourcePoint)
{
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor) return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
	if (!TargetASC) return;

	ApplyToTarget(*TargetASC, HitResult, ResolvedHitEffects, MakeApplicationContext(OwnerAbility, SkillEventScope, StatusEffectSourcePoint));
}

void MASkillDamageApplicator::ApplyToTarget(
	UAbilitySystemComponent& TargetASC,
	const FHitResult& HitResult,
	const FResolvedSkillHitEffects& ResolvedHitEffects,
	const FMASkillDamageApplicationContext& ApplicationContext)
{
	if (ResolvedHitEffects.DamageSpec.IsValid())
	{
		ApplySpecToTargetASC(TargetASC, HitResult, ApplicationContext, ResolvedHitEffects.DamageSpec);
	}

	AActor* HitActor = HitResult.GetActor();
	for (const FResolvedStatusEffect& StatusEffect : ResolvedHitEffects.StatusEffects)
	{
		if (!StatusEffect.SpecHandle.IsValid() || !StatusEffect.SpecHandle.Data.IsValid()) continue;
		if (!ShouldApplyResolvedStatusEffect(TargetASC, HitActor, StatusEffect)) continue;

		FGameplayEffectSpecHandle StatusEffectSpecHandle;
		StatusEffectSpecHandle.Data = MakeShared<FGameplayEffectSpec>(*StatusEffect.SpecHandle.Data.Get());
		UMAAbilitySystemStatics::SetReactionSourcePoint(
			StatusEffectSpecHandle,
			ResolveStatusEffectSourcePoint(ApplicationContext, StatusEffect.SourceType));
		ApplySpecToTargetASC(TargetASC, HitResult, ApplicationContext, StatusEffectSpecHandle);
	}

	ExecuteTargetGameplayCues(TargetASC, HitResult, ResolvedHitEffects, ApplicationContext);
	NotifySkillHit(ApplicationContext, HitResult);
}

void MASkillDamageApplicator::ApplyToTarget(
	UAbilitySystemComponent& TargetASC,
	UMASkillAbility& OwnerAbility,
	UMASkillModuleInstance* SkillEventScope,
	const FHitResult& HitResult,
	const FResolvedSkillHitEffects& ResolvedHitEffects,
	const FVector& StatusEffectSourcePoint)
{
	ApplyToTarget(TargetASC, HitResult, ResolvedHitEffects, MakeApplicationContext(OwnerAbility, SkillEventScope, StatusEffectSourcePoint));
}
