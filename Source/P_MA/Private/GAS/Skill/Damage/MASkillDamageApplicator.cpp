#include "GAS/Skill/Damage/MASkillDamageApplicator.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "GameplayEffectExtension.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Damage/MAGameplayEffect_SkillDamageOverTime.h"
#include "GAS/Skill/Damage/MASkillDamageResolver.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Area/MASkillAreaStatics.h"
#include "GAS/Skill/Area/MASkillAreaTypes.h"
#include "GAS/Skill/Runtime/MASkillRuntimeRegistry.h"
#include "GAS/Skill/Area/Decal/MASkillAreaDecalStatics.h"
#include "GameplayEffect.h"

static FGameplayTag ResolveImpactDamageTypeTag(TConstArrayView<FMASkillDamageConfig> DamageConfigs)
{
	for (const FMASkillDamageConfig& DamageConfig : DamageConfigs)
	{
		if (DamageConfig.HasValues() && DamageConfig.DamageTypeTag.IsValid())
		{
			return DamageConfig.DamageTypeTag;
		}
	}

	return DamageConfigs.IsEmpty() ? FGameplayTag() : DamageConfigs[0].DamageTypeTag;
}

void MASkillDamageApplicator::ApplyArea(
	UMASkillAbility& OwnerAbility,
	const FMASkillScopes& EventScopes,
	const FMASkillWorldAreaShape& Area,
	const FMASkillDamageConfig& DamageConfig,
	const FMASkillPayloadAccessor& Payloads)
{
	ApplyArea(OwnerAbility, EventScopes, Area, MakeArrayView(&DamageConfig, 1), Payloads);
}

void MASkillDamageApplicator::ApplyArea(
	UMASkillAbility& OwnerAbility,
	const FMASkillScopes& EventScopes,
	const FMASkillWorldAreaShape& Area,
	TConstArrayView<FMASkillDamageConfig> DamageConfigs,
	const FMASkillPayloadAccessor& Payloads)
{
	if (!Area.IsValid() || DamageConfigs.IsEmpty()) return;

	MASkillAreaDecalStatics::SpawnImpact(OwnerAbility, Area, ResolveImpactDamageTypeTag(DamageConfigs));
	if (!OwnerAbility.K2_HasAuthority()) return;

	AActor* AvatarActor = OwnerAbility.GetAvatarActorFromActorInfo();
	UWorld* World = OwnerAbility.GetWorld();
	if (!AvatarActor || !World || !Payloads.IsValid()) return;

	if (Area.bDrawDebug) MASkillAreaStatics::DrawWorldPreview(*World, Area);

	TMap<int32, TArray<FHitResult>> HitResultsByTargetRelation;
	for (const FMASkillDamageConfig& DamageConfig : DamageConfigs)
	{
		if (!DamageConfig.HasValues()) continue;

		const FResolvedSkillDamage ResolvedDamage = MASkillDamageResolver::Resolve(OwnerAbility, DamageConfig, Payloads);
		TArray<FHitResult>* HitResults = HitResultsByTargetRelation.Find(ResolvedDamage.TargetRelationMask);
		if (!HitResults)
		{
			HitResults = &HitResultsByTargetRelation.Add(
				ResolvedDamage.TargetRelationMask,
				MASkillAreaStatics::ResolveHitResults(*World, AvatarActor, Area, ResolvedDamage.TargetRelationMask));
		}

		ApplyHitResults(OwnerAbility, EventScopes, *HitResults, ResolvedDamage, Area.Center);
	}
}

MASkillDamageApplicator::FMASkillDamageApplicationContext MASkillDamageApplicator::MakeApplicationContext(
	UMASkillAbility& OwnerAbility,
	const FMASkillScopes& EventScopes,
	const FVector& StatusEffectSourcePoint)
{
	FMASkillDamageApplicationContext Context;
	Context.InstigatorActor = OwnerAbility.GetAvatarActorFromActorInfo();
	Context.EffectCauser = Context.InstigatorActor;
	Context.EventExecutorAbility = &OwnerAbility;
	Context.EventScopes = EventScopes;
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

FActiveGameplayEffectHandle MASkillDamageApplicator::ApplySpecToTargetASC(
	UAbilitySystemComponent& TargetASC,
	const FHitResult& HitResult,
	const FGameplayEffectSpecHandle& SpecHandle)
{
	FGameplayEffectSpecHandle LocalSpecHandle = MakeSpecWithHitResult(HitResult, SpecHandle);
	if (!LocalSpecHandle.IsValid() || !LocalSpecHandle.Data.IsValid()) return FActiveGameplayEffectHandle();

	return TargetASC.ApplyGameplayEffectSpecToSelf(*LocalSpecHandle.Data.Get());
}

bool MASkillDamageApplicator::ApplyDamageSpecToTargetASC(
	UAbilitySystemComponent& TargetASC,
	const FHitResult& HitResult,
	const FResolvedSkillDamage& ResolvedDamage,
	const FMASkillDamageApplicationContext& ApplicationContext,
	FGameplayEffectContextHandle& OutContextHandle)
{
	FGameplayEffectSpecHandle LocalSpecHandle = MakeSpecWithHitResult(HitResult, ResolvedDamage.DamageSpec);
	if (!LocalSpecHandle.IsValid() || !LocalSpecHandle.Data.IsValid()) return false;

	FGameplayEffectContextHandle ContextHandle = LocalSpecHandle.Data->GetContext();
	ContextHandle.AddInstigator(ApplicationContext.InstigatorActor, ApplicationContext.EffectCauser);
	if (ApplicationContext.EventScopes.Module)
	{
		ContextHandle.AddSourceObject(ApplicationContext.EventScopes.Module.Get());
	}
	if (ApplicationContext.EventExecutorAbility)
	{
		ContextHandle.SetAbility(ApplicationContext.EventExecutorAbility);
	}
	if (ResolvedDamage.ApplicationMode == EMASkillDamageApplicationMode::DamageOverTime)
	{
		LocalSpecHandle.Data->AppendDynamicAssetTags(ResolvedDamage.TargetGameplayCueTags);
	}

	const FActiveGameplayEffectHandle EffectHandle = TargetASC.ApplyGameplayEffectSpecToSelf(*LocalSpecHandle.Data.Get());
	ApplicationContext.EventScopes.GetRuntimeRegistry().Register(&TargetASC, EffectHandle);
	OutContextHandle = ContextHandle;

	const FMAGameplayEffectContext* MAContext = static_cast<const FMAGameplayEffectContext*>(ContextHandle.Get());
	return MAContext && MAContext->GetDamageTypeTag().IsValid();
}

void MASkillDamageApplicator::ExecuteTargetGameplayCues(
	UAbilitySystemComponent& TargetASC,
	const FHitResult& HitResult,
	const FGameplayTagContainer& GameplayCueTags,
	const FMASkillDamageApplicationContext& ApplicationContext)
{
	if (GameplayCueTags.IsEmpty()) return;

	FGameplayCueParameters CueParams;
	CueParams.Location = HitResult.ImpactPoint.IsNearlyZero() ? HitResult.Location : HitResult.ImpactPoint;
	CueParams.Normal = HitResult.ImpactNormal.IsNearlyZero() ? HitResult.Normal : HitResult.ImpactNormal;
	CueParams.Instigator = ApplicationContext.InstigatorActor;
	CueParams.EffectCauser = ApplicationContext.EffectCauser ? ApplicationContext.EffectCauser : ApplicationContext.InstigatorActor;

	for (const FGameplayTag& GameplayCueTag : GameplayCueTags)
	{
		if (!GameplayCueTag.IsValid()) continue;

		FGameplayCueParameters CueParamsForTag = CueParams;
		CueParamsForTag.OriginalTag = GameplayCueTag;
		CueParamsForTag.AggregatedSourceTags.AddTag(GameplayCueTag);
		CueParamsForTag.AggregatedTargetTags.AddTag(GameplayCueTag);
		TargetASC.ExecuteGameplayCue(GameplayCueTag, CueParamsForTag);
	}
}

bool MASkillDamageApplicator::PostProcessDamage(
	UAbilitySystemComponent& TargetASC,
	const FGameplayEffectContextHandle& ContextHandle,
	const FHitResult& HitResult,
	AActor* TargetActor,
	const FGameplayTagContainer& TargetGameplayCueTags,
	const FMASkillDamageApplicationContext& ApplicationContext,
	FMASkillEvent& OutHitEvent)
{
	const FMAGameplayEffectContext* MAContext = static_cast<const FMAGameplayEffectContext*>(ContextHandle.Get());
	if (!MAContext || !MAContext->GetDamageTypeTag().IsValid()) return false;

	if (MAContext->GetDisplayMagnitude() > 0.f)
	{
		ExecuteTargetGameplayCues(
			TargetASC,
			HitResult,
			TargetGameplayCueTags,
			ApplicationContext);
	}

	if (MAContext->GetDamageTypeTag() != UMAAbilitySystemStatics::GetDefaultDamageTypeTag()
		|| MAContext->GetDisplayMagnitude() <= 0.f)
	{
		return false;
	}

	OutHitEvent = FMASkillEvent(UMAAbilitySystemStatics::GetHitEventTag(), ApplicationContext.EventScopes);
	OutHitEvent.Payloads.SetObject(UMAAbilitySystemStatics::GetDamageTargetTag(), TargetActor);
	OutHitEvent.Payloads.SetScalar(UMAAbilitySystemStatics::GetAppliedDamageTag(), MAContext->GetDisplayMagnitude());
	return true;
}

void MASkillDamageApplicator::PostProcessAppliedDamage(
	UAbilitySystemComponent& TargetASC,
	const FGameplayEffectModCallbackData& Data)
{
	const UGameplayEffect* EffectDefinition = Data.EffectSpec.Def.Get();
	if (!EffectDefinition || !EffectDefinition->IsA(UMAGameplayEffect_SkillDamageOverTime::StaticClass())) return;

	const FGameplayEffectContextHandle ContextHandle = Data.EffectSpec.GetContext();
	const FMAGameplayEffectContext* MAContext = static_cast<const FMAGameplayEffectContext*>(ContextHandle.Get());
	if (!MAContext) return;

	UMASkillAbility* ExecutorAbility = const_cast<UMASkillAbility*>(
		Cast<UMASkillAbility>(ContextHandle.GetAbilityInstance_NotReplicated()));
	UMASkillModuleInstance* SkillScope = ExecutorAbility ? ExecutorAbility->GetCurrentSkillModuleInstance() : nullptr;
	if (!ExecutorAbility || !SkillScope) return;

	FMASkillDamageApplicationContext ApplicationContext;
	ApplicationContext.InstigatorActor = ContextHandle.GetOriginalInstigator();
	ApplicationContext.EffectCauser = ContextHandle.GetEffectCauser();
	ApplicationContext.EventExecutorAbility = ExecutorAbility;
	ApplicationContext.EventScopes = FMASkillScopes(Cast<UMASkillModuleInstance>(ContextHandle.GetSourceObject()), SkillScope);

	AActor* TargetActor = Data.Target.AbilityActorInfo ? Data.Target.AbilityActorInfo->AvatarActor.Get() : nullptr;
	FHitResult HitResult;
	if (const FHitResult* ContextHitResult = ContextHandle.GetHitResult())
	{
		HitResult = *ContextHitResult;
	}
	if (TargetActor)
	{
		const FVector TargetLocation = TargetActor->GetActorLocation();
		HitResult.Location = TargetLocation;
		HitResult.ImpactPoint = TargetLocation;
	}

	FGameplayTagContainer TargetGameplayCueTags;
	const FGameplayTag HitGameplayCueRootTag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Hit"));
	for (const FGameplayTag& GameplayCueTag : Data.EffectSpec.GetDynamicAssetTags())
	{
		if (GameplayCueTag.MatchesTag(HitGameplayCueRootTag))
		{
			TargetGameplayCueTags.AddTag(GameplayCueTag);
		}
	}

	FMASkillEvent HitEvent;
	if (PostProcessDamage(
		TargetASC,
		ContextHandle,
		HitResult,
		TargetActor,
		TargetGameplayCueTags,
		ApplicationContext,
		HitEvent))
	{
		UMASkillEventRoutingStatics::TryNotifySkillEvent(ExecutorAbility, MoveTemp(HitEvent));
	}
}

void MASkillDamageApplicator::ApplyHitResults(
	UMASkillAbility& OwnerAbility,
	const FMASkillScopes& EventScopes,
	const TArray<FHitResult>& HitResults,
	const FResolvedSkillDamage& ResolvedDamage,
	const FVector& StatusEffectSourcePoint)
{
	TSet<AActor*> HitActors;
	const FMASkillDamageApplicationContext ApplicationContext = MakeApplicationContext(OwnerAbility, EventScopes, StatusEffectSourcePoint);
	TArray<FMASkillEvent> HitEvents;
	HitEvents.Reserve(HitResults.Num());

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

		FMASkillEvent HitEvent;
		if (ApplyToTargetInternal(*TargetASC, HitResult, ResolvedDamage, ApplicationContext, HitEvent))
		{
			HitEvents.Add(MoveTemp(HitEvent));
		}
		HitActors.Add(HitActor);
	}

	if (!HitEvents.IsEmpty())
	{
		UMASkillEventRoutingStatics::TryNotifySkillEventGroup(&OwnerAbility, MoveTemp(HitEvents));
	}
}

void MASkillDamageApplicator::ApplyToTargetActor(
	UMASkillAbility& OwnerAbility,
	const FMASkillScopes& EventScopes,
	AActor& TargetActor,
	const FResolvedSkillDamage& ResolvedDamage,
	const FVector& StatusEffectSourcePoint)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(&TargetActor);
	if (!TargetASC) return;

	const FVector TargetLocation = TargetActor.GetActorLocation();
	FHitResult HitResult(&TargetActor, nullptr, TargetLocation, FVector::UpVector);

	ApplyToTarget(*TargetASC, HitResult, ResolvedDamage, MakeApplicationContext(OwnerAbility, EventScopes, StatusEffectSourcePoint));
}

void MASkillDamageApplicator::ApplyToTarget(
	UAbilitySystemComponent& TargetASC,
	const FHitResult& HitResult,
	const FResolvedSkillDamage& ResolvedDamage,
	const FMASkillDamageApplicationContext& ApplicationContext)
{
	FMASkillEvent HitEvent;
	if (!ApplyToTargetInternal(TargetASC, HitResult, ResolvedDamage, ApplicationContext, HitEvent)) return;

	UMASkillEventRoutingStatics::TryNotifySkillEvent(
		ApplicationContext.EventExecutorAbility,
		MoveTemp(HitEvent));
}

bool MASkillDamageApplicator::ApplyToTargetInternal(
	UAbilitySystemComponent& TargetASC,
	const FHitResult& HitResult,
	const FResolvedSkillDamage& ResolvedDamage,
	const FMASkillDamageApplicationContext& ApplicationContext,
	FMASkillEvent& OutHitEvent)
{
	bool bHasHitEvent = false;
	if (ResolvedDamage.DamageSpec.IsValid())
	{
		FGameplayEffectContextHandle ContextHandle;
		const bool bHasDamageContext = ApplyDamageSpecToTargetASC(
			TargetASC,
			HitResult,
			ResolvedDamage,
			ApplicationContext,
			ContextHandle);
		if (bHasDamageContext && ResolvedDamage.ApplicationMode == EMASkillDamageApplicationMode::Instant)
		{
			bHasHitEvent = PostProcessDamage(
				TargetASC,
				ContextHandle,
				HitResult,
				TargetASC.GetAvatarActor(),
				ResolvedDamage.TargetGameplayCueTags,
				ApplicationContext,
				OutHitEvent);
		}
	}

	ApplyStatusEffects(TargetASC, HitResult, ResolvedDamage, ApplicationContext);
	return bHasHitEvent;
}

void MASkillDamageApplicator::ApplyStatusEffects(
	UAbilitySystemComponent& TargetASC,
	const FHitResult& HitResult,
	const FResolvedSkillDamage& ResolvedDamage,
	const FMASkillDamageApplicationContext& ApplicationContext)
{
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
		const FActiveGameplayEffectHandle EffectHandle =
			ApplySpecToTargetASC(TargetASC, HitResult, StatusEffectSpecHandle);
		ApplicationContext.EventScopes.GetRuntimeRegistry().Register(&TargetASC, EffectHandle);
	}
}

void MASkillDamageApplicator::ApplyToTarget(
	UAbilitySystemComponent& TargetASC,
	UMASkillAbility& OwnerAbility,
	const FMASkillScopes& EventScopes,
	const FHitResult& HitResult,
	const FResolvedSkillDamage& ResolvedDamage,
	const FVector& StatusEffectSourcePoint)
{
	ApplyToTarget(TargetASC, HitResult, ResolvedDamage, MakeApplicationContext(OwnerAbility, EventScopes, StatusEffectSourcePoint));
}
