#include "GAS/Skill/Damage/MASkillDamageApplicator.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Engine/HitResult.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/MAAbilitySystemComponent.h"
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

FGameplayEffectSpecHandle MASkillDamageApplicator::MakeSpecWithHitResult(
	const FHitResult& HitResult,
	const FGameplayEffectSpecHandle& SpecHandle)
{
	if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
	{
		return FGameplayEffectSpecHandle();
	}

	FGameplayEffectSpecHandle LocalSpecHandle;
	LocalSpecHandle.Data = MakeShared<FGameplayEffectSpec>(*SpecHandle.Data.Get());

	FGameplayEffectContextHandle ContextHandle = LocalSpecHandle.Data->GetContext().Duplicate();
	ContextHandle.AddHitResult(HitResult, true);
	LocalSpecHandle.Data->SetContext(ContextHandle);
	return LocalSpecHandle;
}

void MASkillDamageApplicator::ApplySpecToTargetASC(
	UAbilitySystemComponent& TargetASC,
	const FHitResult& HitResult,
	const FGameplayEffectSpecHandle& SpecHandle)
{
	FGameplayEffectSpecHandle LocalSpecHandle = MakeSpecWithHitResult(HitResult, SpecHandle);
	if (!LocalSpecHandle.IsValid() || !LocalSpecHandle.Data.IsValid()) return;

	TargetASC.ApplyGameplayEffectSpecToSelf(*LocalSpecHandle.Data.Get());
}

bool MASkillDamageApplicator::ApplyDamageSpecToTargetASC(
	UAbilitySystemComponent& TargetASC,
	const FHitResult& HitResult,
	const FGameplayEffectSpecHandle& SpecHandle,
	FMADamageAppliedEvent& OutDamageAppliedEvent)
{
	FGameplayEffectSpecHandle LocalSpecHandle = MakeSpecWithHitResult(HitResult, SpecHandle);
	if (!LocalSpecHandle.IsValid() || !LocalSpecHandle.Data.IsValid()) return false;

	TargetASC.ApplyGameplayEffectSpecToSelf(*LocalSpecHandle.Data.Get());

	const FGameplayEffectContextHandle ContextHandle = LocalSpecHandle.Data->GetContext();
	const FMAGameplayEffectContext* MAContext = static_cast<const FMAGameplayEffectContext*>(ContextHandle.Get());
	if (!MAContext || MAContext->GetDisplayMagnitude() <= 0.f) return false;

	const FGameplayTag DamageTypeTag = MAContext->GetDamageTypeTag().IsValid()
		? MAContext->GetDamageTypeTag()
		: UMAAbilitySystemStatics::GetDefaultDamageTypeTag();

	FMADamageAppliedEvent DamageAppliedEvent;
	DamageAppliedEvent.SourceActor = ContextHandle.GetOriginalInstigator();
	DamageAppliedEvent.TargetActor = TargetASC.GetAvatarActor();
	DamageAppliedEvent.HitResult = HitResult;
	DamageAppliedEvent.DisplayMagnitude = MAContext->GetDisplayMagnitude();
	DamageAppliedEvent.DamageTypeTag = DamageTypeTag;
	DamageAppliedEvent.CriticalResult = MAContext->GetCriticalResult();

	OutDamageAppliedEvent = DamageAppliedEvent;
	return true;
}

void MASkillDamageApplicator::ExecuteTargetGameplayCues(
	UAbilitySystemComponent& TargetASC,
	const FHitResult& HitResult,
	const FResolvedSkillDamage& ResolvedDamage,
	const FMASkillDamageApplicationContext& ApplicationContext)
{
	if (ResolvedDamage.TargetGameplayCueTags.IsEmpty()) return;

	FGameplayCueParameters CueParams;
	CueParams.Location = HitResult.ImpactPoint.IsNearlyZero() ? HitResult.Location : HitResult.ImpactPoint;
	CueParams.Normal = HitResult.ImpactNormal.IsNearlyZero() ? HitResult.Normal : HitResult.ImpactNormal;
	CueParams.Instigator = ApplicationContext.InstigatorActor;
	CueParams.EffectCauser = ApplicationContext.EffectCauser ? ApplicationContext.EffectCauser : ApplicationContext.InstigatorActor;

	for (const FGameplayTag& GameplayCueTag : ResolvedDamage.TargetGameplayCueTags)
	{
		if (!GameplayCueTag.IsValid()) continue;

		FGameplayCueParameters CueParamsForTag = CueParams;
		CueParamsForTag.OriginalTag = GameplayCueTag;
		CueParamsForTag.AggregatedSourceTags.AddTag(GameplayCueTag);
		CueParamsForTag.AggregatedTargetTags.AddTag(GameplayCueTag);
		TargetASC.ExecuteGameplayCue(GameplayCueTag, CueParamsForTag);
	}
}

void MASkillDamageApplicator::ApplyHitResults(
	UMASkillAbility& OwnerAbility,
	UMASkillModuleInstance* SkillEventScope,
	const TArray<FHitResult>& HitResults,
	const FResolvedSkillDamage& ResolvedDamage,
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

		ApplyToTarget(*TargetASC, HitResult, ResolvedDamage, ApplicationContext);
		HitActors.Add(HitActor);
	}
}

void MASkillDamageApplicator::ApplyHitResult(
	UMASkillAbility& OwnerAbility,
	UMASkillModuleInstance* SkillEventScope,
	const FHitResult& HitResult,
	const FResolvedSkillDamage& ResolvedDamage,
	const FVector& StatusEffectSourcePoint)
{
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor) return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
	if (!TargetASC) return;

	ApplyToTarget(*TargetASC, HitResult, ResolvedDamage, MakeApplicationContext(OwnerAbility, SkillEventScope, StatusEffectSourcePoint));
}

void MASkillDamageApplicator::ApplyToTargetActor(
	UMASkillAbility& OwnerAbility,
	UMASkillModuleInstance* SkillEventScope,
	AActor& TargetActor,
	const FResolvedSkillDamage& ResolvedDamage,
	const FVector& StatusEffectSourcePoint)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(&TargetActor);
	if (!TargetASC) return;

	const FVector TargetLocation = TargetActor.GetActorLocation();
	FHitResult HitResult(&TargetActor, nullptr, TargetLocation, FVector::UpVector);

	ApplyToTarget(*TargetASC, HitResult, ResolvedDamage, MakeApplicationContext(OwnerAbility, SkillEventScope, StatusEffectSourcePoint));
}

void MASkillDamageApplicator::ApplyToTarget(
	UAbilitySystemComponent& TargetASC,
	const FHitResult& HitResult,
	const FResolvedSkillDamage& ResolvedDamage,
	const FMASkillDamageApplicationContext& ApplicationContext)
{
	FMADamageAppliedEvent DamageAppliedEvent;
	bool bHasDamageAppliedEvent = false;
	if (ResolvedDamage.DamageSpec.IsValid())
	{
		bHasDamageAppliedEvent = ApplyDamageSpecToTargetASC(
			TargetASC,
			HitResult,
			ResolvedDamage.DamageSpec,
			DamageAppliedEvent);
	}

	AActor* HitActor = HitResult.GetActor();
	for (const FResolvedStatusEffect& StatusEffect : ResolvedDamage.StatusEffects)
	{
		if (!StatusEffect.SpecHandle.IsValid() || !StatusEffect.SpecHandle.Data.IsValid()) continue;
		if (!ShouldApplyResolvedStatusEffect(TargetASC, HitActor, StatusEffect)) continue;

		FGameplayEffectSpecHandle StatusEffectSpecHandle;
		StatusEffectSpecHandle.Data = MakeShared<FGameplayEffectSpec>(*StatusEffect.SpecHandle.Data.Get());
		UMAAbilitySystemStatics::SetReactionSourcePoint(
			StatusEffectSpecHandle,
			ResolveStatusEffectSourcePoint(ApplicationContext, StatusEffect.SourceType));
		ApplySpecToTargetASC(TargetASC, HitResult, StatusEffectSpecHandle);
	}

	ExecuteTargetGameplayCues(TargetASC, HitResult, ResolvedDamage, ApplicationContext);

	if (bHasDamageAppliedEvent
		&& DamageAppliedEvent.DamageTypeTag.MatchesTag(UMAAbilitySystemStatics::GetDefaultDamageTypeTag())
		&& ApplicationContext.SkillEventScope)
	{
		FGameplayEventData EventData;
		EventData.Instigator = DamageAppliedEvent.SourceActor.Get();
		EventData.Target = DamageAppliedEvent.TargetActor.Get();
		EventData.EventMagnitude = DamageAppliedEvent.DisplayMagnitude;
		if (DamageAppliedEvent.HitResult.GetActor())
		{
			EventData.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(DamageAppliedEvent.HitResult);
		}
		ApplicationContext.SkillEventScope->BroadcastScopedEvent(UMAAbilitySystemStatics::GetDamageDealtEventTag(), EventData);
	}
}

void MASkillDamageApplicator::ApplyToTarget(
	UAbilitySystemComponent& TargetASC,
	UMASkillAbility& OwnerAbility,
	UMASkillModuleInstance* SkillEventScope,
	const FHitResult& HitResult,
	const FResolvedSkillDamage& ResolvedDamage,
	const FVector& StatusEffectSourcePoint)
{
	ApplyToTarget(TargetASC, HitResult, ResolvedDamage, MakeApplicationContext(OwnerAbility, SkillEventScope, StatusEffectSourcePoint));
}
