#include "GAS/Skill/Damage/MADamageApplicator.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Area/Decal/MASkillAreaDecalStatics.h"
#include "GAS/Skill/Area/MASkillAreaStatics.h"
#include "GAS/Skill/Area/MASkillAreaTypes.h"
#include "GAS/Skill/Damage/MASkillDamageResolver.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Runtime/MASkillRuntimeRegistry.h"

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

void MADamageApplicator::ApplyArea(
	UMASkillAbility& OwnerAbility,
	const FMASkillScopes& EventScopes,
	const FMASkillWorldAreaShape& Area,
	const FMASkillDamageConfig& DamageConfig,
	const FMASkillPayloadAccess& Payloads)
{
	ApplyArea(OwnerAbility, EventScopes, Area, MakeArrayView(&DamageConfig, 1), Payloads);
}

void MADamageApplicator::ApplyArea(
	UMASkillAbility& OwnerAbility,
	const FMASkillScopes& EventScopes,
	const FMASkillWorldAreaShape& Area,
	TConstArrayView<FMASkillDamageConfig> DamageConfigs,
	const FMASkillPayloadAccess& Payloads)
{
	if (!Area.IsValid() || DamageConfigs.IsEmpty()) return;

	MASkillAreaDecalStatics::SpawnImpact(OwnerAbility, Area, ResolveImpactDamageTypeTag(DamageConfigs));
	if (!OwnerAbility.K2_HasAuthority()) return;

	AActor* SourceActor = OwnerAbility.GetAvatarActorFromActorInfo();
	UWorld* World = OwnerAbility.GetWorld();
	if (!SourceActor || !World || !Payloads.Reader.IsValid()) return;

	if (Area.bDrawDebug) MASkillAreaStatics::DrawWorldPreview(*World, Area);

	TMap<int32, TArray<FHitResult>> HitResultsByTargetRelation;
	for (const FMASkillDamageConfig& DamageConfig : DamageConfigs)
	{
		if (!DamageConfig.HasValues()) continue;

		const FMAResolvedDamage ResolvedDamage = MASkillDamageResolver::Resolve(
			OwnerAbility,
			EventScopes,
			DamageConfig,
			Payloads);
		TArray<FHitResult>* HitResults = HitResultsByTargetRelation.Find(ResolvedDamage.TargetRelationMask);
		if (!HitResults)
		{
			HitResults = &HitResultsByTargetRelation.Add(
				ResolvedDamage.TargetRelationMask,
				MASkillAreaStatics::ResolveHitResults(*World, SourceActor, Area, ResolvedDamage.TargetRelationMask));
		}

		ApplyHitResults(*HitResults, ResolvedDamage, Area.Center);
	}
}

void MADamageApplicator::ApplyArea(
	const FGameplayEffectContextHandle& SourceContext,
	AActor& AreaOwner,
	const FMASkillWorldAreaShape& Area,
	TConstArrayView<FMASkillDamageConfig> DamageConfigs)
{
	if (!Area.IsValid() || DamageConfigs.IsEmpty()) return;

	MASkillAreaDecalStatics::SpawnImpact(AreaOwner, Area, ResolveImpactDamageTypeTag(DamageConfigs));
	if (!AreaOwner.HasAuthority()) return;

	UWorld* World = AreaOwner.GetWorld();
	UAbilitySystemComponent* SourceASC = SourceContext.GetOriginalInstigatorAbilitySystemComponent();
	if (!World || !SourceASC) return;

	AActor* SourceActor = SourceContext.GetOriginalInstigator();
	if (!SourceActor) SourceActor = SourceASC->GetAvatarActor();
	if (!SourceActor) SourceActor = SourceASC->GetOwnerActor();
	if (!SourceActor) return;

	if (Area.bDrawDebug) MASkillAreaStatics::DrawWorldPreview(*World, Area);

	TMap<int32, TArray<FHitResult>> HitResultsByTargetRelation;
	for (const FMASkillDamageConfig& DamageConfig : DamageConfigs)
	{
		if (!DamageConfig.HasValues()) continue;

		const FMAResolvedDamage ResolvedDamage = MASkillDamageResolver::Resolve(
			*SourceASC,
			SourceContext,
			DamageConfig);
		TArray<FHitResult>* HitResults = HitResultsByTargetRelation.Find(ResolvedDamage.TargetRelationMask);
		if (!HitResults)
		{
			HitResults = &HitResultsByTargetRelation.Add(
				ResolvedDamage.TargetRelationMask,
				MASkillAreaStatics::ResolveHitResults(*World, SourceActor, Area, ResolvedDamage.TargetRelationMask));
		}

		ApplyHitResults(*HitResults, ResolvedDamage, Area.Center);
	}
}

FVector MADamageApplicator::ResolveStatusEffectSourcePoint(
	const FGameplayEffectContextHandle& ContextHandle,
	const FVector& StatusEffectSourcePoint,
	EMASkillStatusEffectSourceType SourceType)
{
	if (SourceType == EMASkillStatusEffectSourceType::Center)
	{
		return StatusEffectSourcePoint;
	}

	const AActor* InstigatorActor = ContextHandle.GetOriginalInstigator();
	return InstigatorActor ? InstigatorActor->GetActorLocation() : StatusEffectSourcePoint;
}

bool MADamageApplicator::ResolveSkillEventSource(
	const FGameplayEffectContextHandle& ContextHandle,
	UMASkillAbility*& OutAbility,
	FMASkillScopes& OutScopes)
{
	OutAbility = const_cast<UMASkillAbility*>(
		Cast<UMASkillAbility>(ContextHandle.GetAbilityInstance_NotReplicated()));
	const FMAGameplayEffectContext* MAContext = static_cast<const FMAGameplayEffectContext*>(ContextHandle.Get());
	OutScopes = FMASkillScopes(
		Cast<UMASkillModuleInstance>(ContextHandle.GetSourceObject()),
		MAContext ? MAContext->GetSkillScope() : nullptr);
	return OutAbility && OutScopes.Module && OutScopes.Skill;
}

void MADamageApplicator::RegisterWithSkillRuntime(
	const FGameplayEffectContextHandle& ContextHandle,
	UAbilitySystemComponent& TargetASC,
	const FActiveGameplayEffectHandle& EffectHandle)
{
	if (!EffectHandle.IsValid()) return;

	const FMAGameplayEffectContext* MAContext = static_cast<const FMAGameplayEffectContext*>(ContextHandle.Get());
	if (UMASkillModuleInstance* SkillScope = MAContext ? MAContext->GetSkillScope() : nullptr)
	{
		if (UMASkillRuntimeRegistry* RuntimeRegistry = SkillScope->GetRuntimeRegistry())
		{
			RuntimeRegistry->Register(&TargetASC, EffectHandle);
		}
	}
}

bool MADamageApplicator::ShouldApplyResolvedStatusEffect(
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
	if (ActiveEffects.IsEmpty()) return true;

	for (const FActiveGameplayEffectHandle ActiveEffectHandle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveEffect = TargetASC.GetActiveGameplayEffect(ActiveEffectHandle);
		if (!ActiveEffect) continue;

		const float ExistingStrengthMagnitude = ActiveEffect->Spec.GetSetByCallerMagnitude(
			FGameplayTag::RequestGameplayTag(TEXT("Data.StatusEffect.StrengthMagnitude")),
			false,
			StatusEffect.StrengthMagnitude);
		if (StatusEffect.StrengthPolicy == EMASkillStatusEffectStrengthPolicy::LargerMagnitudeStronger
			&& ExistingStrengthMagnitude > StatusEffect.StrengthMagnitude
			&& !FMath::IsNearlyEqual(ExistingStrengthMagnitude, StatusEffect.StrengthMagnitude))
		{
			return false;
		}
		if (StatusEffect.StrengthPolicy == EMASkillStatusEffectStrengthPolicy::SmallerMagnitudeStronger
			&& ExistingStrengthMagnitude < StatusEffect.StrengthMagnitude
			&& !FMath::IsNearlyEqual(ExistingStrengthMagnitude, StatusEffect.StrengthMagnitude))
		{
			return false;
		}
	}

	for (const FActiveGameplayEffectHandle ActiveEffectHandle : ActiveEffects)
	{
		TargetASC.RemoveActiveGameplayEffect(ActiveEffectHandle);
	}
	return true;
}

FGameplayEffectSpecHandle MADamageApplicator::MakeSpecWithHitResult(
	const FHitResult& HitResult,
	const FGameplayEffectSpecHandle& SpecHandle)
{
	if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid()) return FGameplayEffectSpecHandle();

	FGameplayEffectSpecHandle LocalSpecHandle;
	LocalSpecHandle.Data = MakeShared<FGameplayEffectSpec>(*SpecHandle.Data.Get());
	FGameplayEffectContextHandle ContextHandle = LocalSpecHandle.Data->GetContext().Duplicate();
	ContextHandle.AddHitResult(HitResult, true);
	LocalSpecHandle.Data->SetContext(ContextHandle);
	return LocalSpecHandle;
}

bool MADamageApplicator::ApplyDamageSpecToTargetASC(
	UAbilitySystemComponent& TargetASC,
	const FHitResult& HitResult,
	const FMAResolvedDamage& ResolvedDamage,
	FGameplayEffectContextHandle& OutContextHandle)
{
	FGameplayEffectSpecHandle LocalSpecHandle = MakeSpecWithHitResult(HitResult, ResolvedDamage.DamageSpec);
	if (!LocalSpecHandle.IsValid() || !LocalSpecHandle.Data.IsValid()) return false;

	if (ResolvedDamage.ApplicationMode == EMASkillDamageApplicationMode::DamageOverTime)
	{
		LocalSpecHandle.Data->AppendDynamicAssetTags(ResolvedDamage.TargetGameplayCueTags);
	}

	const FActiveGameplayEffectHandle EffectHandle = TargetASC.ApplyGameplayEffectSpecToSelf(*LocalSpecHandle.Data.Get());
	OutContextHandle = LocalSpecHandle.Data->GetContext();
	RegisterWithSkillRuntime(OutContextHandle, TargetASC, EffectHandle);

	const FMAGameplayEffectContext* MAContext = static_cast<const FMAGameplayEffectContext*>(OutContextHandle.Get());
	return MAContext && MAContext->GetDamageTypeTag().IsValid();
}

void MADamageApplicator::ExecuteTargetGameplayCues(
	UAbilitySystemComponent& TargetASC,
	const FHitResult& HitResult,
	const FGameplayTagContainer& GameplayCueTags,
	const FGameplayEffectContextHandle& ContextHandle)
{
	if (GameplayCueTags.IsEmpty()) return;

	FGameplayCueParameters CueParams;
	CueParams.Location = HitResult.ImpactPoint.IsNearlyZero() ? HitResult.Location : HitResult.ImpactPoint;
	CueParams.Normal = HitResult.ImpactNormal.IsNearlyZero() ? HitResult.Normal : HitResult.ImpactNormal;
	CueParams.Instigator = ContextHandle.GetOriginalInstigator();
	CueParams.EffectCauser = ContextHandle.GetEffectCauser();

	UMAAbilitySystemComponent* MATargetASC = Cast<UMAAbilitySystemComponent>(&TargetASC);
	checkf(MATargetASC, TEXT("MA damage targets must use UMAAbilitySystemComponent."));
	if (!MATargetASC) return;

	MATargetASC->ExecuteGameplayCues(GameplayCueTags, CueParams);
}

bool MADamageApplicator::PostProcessDamage(
	UAbilitySystemComponent& TargetASC,
	const FGameplayEffectContextHandle& ContextHandle,
	const FHitResult& HitResult,
	AActor* TargetActor,
	const FGameplayTagContainer& TargetGameplayCueTags,
	FMASkillEvent& OutHitEvent)
{
	const FMAGameplayEffectContext* MAContext = static_cast<const FMAGameplayEffectContext*>(ContextHandle.Get());
	if (!MAContext || !MAContext->GetDamageTypeTag().IsValid()) return false;

	if (MAContext->GetDisplayMagnitude() > 0.f)
	{
		ExecuteTargetGameplayCues(TargetASC, HitResult, TargetGameplayCueTags, ContextHandle);
	}

	if (MAContext->GetDamageTypeTag() != UMAAbilitySystemStatics::GetDefaultDamageTypeTag()
		|| MAContext->GetDisplayMagnitude() <= 0.f)
	{
		return false;
	}

	UMASkillAbility* ExecutorAbility = nullptr;
	FMASkillScopes EventScopes;
	if (!ResolveSkillEventSource(ContextHandle, ExecutorAbility, EventScopes)) return false;

	OutHitEvent = FMASkillEvent(UMAAbilitySystemStatics::GetHitEventTag(), EventScopes);
	OutHitEvent.Payloads.SetObject(UMAAbilitySystemStatics::GetDamageTargetTag(), TargetActor);
	OutHitEvent.Payloads.SetScalar(UMAAbilitySystemStatics::GetAppliedDamageTag(), MAContext->GetDisplayMagnitude());
	return true;
}

void MADamageApplicator::PostProcessAppliedDamage(
	UAbilitySystemComponent& TargetASC,
	const FGameplayEffectModCallbackData& Data)
{
	const FGameplayEffectContextHandle ContextHandle = Data.EffectSpec.GetContext();
	const FMAGameplayEffectContext* MAContext = static_cast<const FMAGameplayEffectContext*>(ContextHandle.Get());
	AActor* TargetActor = Data.Target.AbilityActorInfo ? Data.Target.AbilityActorInfo->AvatarActor.Get() : nullptr;
	if (MAContext && TargetActor && MAContext->GetDisplayMagnitude() > 0.f)
	{
		UMASkillAbility* ExecutorAbility = nullptr;
		FMASkillScopes EventScopes;
		if (ResolveSkillEventSource(ContextHandle, ExecutorAbility, EventScopes))
		{
			FMASkillEvent DamageAppliedEvent(UMAAbilitySystemStatics::GetDamageAppliedEventTag(), EventScopes);
			DamageAppliedEvent.Payloads.SetObject(UMAAbilitySystemStatics::GetDamageTargetTag(), TargetActor);
			DamageAppliedEvent.Payloads.SetScalar(
				UMAAbilitySystemStatics::GetAppliedDamageTag(),
				MAContext->GetDisplayMagnitude());
			UMASkillEventRoutingStatics::TryNotifySkillEvent(ExecutorAbility, MoveTemp(DamageAppliedEvent));
		}
	}

	if (Data.EffectSpec.GetPeriod() <= 0.f) return;

	FHitResult HitResult;
	if (const FHitResult* ContextHitResult = ContextHandle.GetHitResult())
	{
		HitResult = *ContextHitResult;
	}
	if (TargetActor)
	{
		HitResult.Location = TargetActor->GetActorLocation();
		HitResult.ImpactPoint = HitResult.Location;
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
	if (!PostProcessDamage(
		TargetASC,
		ContextHandle,
		HitResult,
		TargetActor,
		TargetGameplayCueTags,
		HitEvent))
	{
		return;
	}

	UMASkillAbility* ExecutorAbility = nullptr;
	FMASkillScopes EventScopes;
	if (ResolveSkillEventSource(ContextHandle, ExecutorAbility, EventScopes))
	{
		UMASkillEventRoutingStatics::TryNotifySkillEvent(ExecutorAbility, MoveTemp(HitEvent));
	}
}

void MADamageApplicator::NotifyTargetKilled(
	UAbilitySystemComponent& TargetASC,
	const FGameplayEffectSpec& KillingEffectSpec)
{
	const FGameplayEffectContextHandle ContextHandle = KillingEffectSpec.GetContext();
	const FMAGameplayEffectContext* MAContext = static_cast<const FMAGameplayEffectContext*>(ContextHandle.Get());
	if (!MAContext || !MAContext->GetDamageTypeTag().IsValid() || MAContext->GetDisplayMagnitude() <= 0.f) return;

	UMASkillAbility* ExecutorAbility = nullptr;
	FMASkillScopes EventScopes;
	AActor* TargetActor = TargetASC.GetAvatarActor();
	if (!TargetActor || !ResolveSkillEventSource(ContextHandle, ExecutorAbility, EventScopes)) return;

	FMASkillEvent KillEvent(UMAAbilitySystemStatics::GetKillEventTag(), EventScopes);
	KillEvent.Payloads.SetObject(UMAAbilitySystemStatics::GetDamageTargetTag(), TargetActor);
	KillEvent.Payloads.SetScalar(UMAAbilitySystemStatics::GetAppliedDamageTag(), MAContext->GetDisplayMagnitude());
	UMASkillEventRoutingStatics::TryNotifySkillEvent(ExecutorAbility, MoveTemp(KillEvent));
}

void MADamageApplicator::ApplyHitResults(
	const TArray<FHitResult>& HitResults,
	const FMAResolvedDamage& ResolvedDamage,
	const FVector& StatusEffectSourcePoint)
{
	TSet<AActor*> HitActors;
	TArray<FMASkillEvent> HitEvents;
	HitEvents.Reserve(HitResults.Num());

	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor || HitActors.Contains(HitActor)) continue;

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!TargetASC) continue;

		FMASkillEvent HitEvent;
		if (ApplyToTargetInternal(*TargetASC, HitResult, ResolvedDamage, StatusEffectSourcePoint, HitEvent))
		{
			HitEvents.Add(MoveTemp(HitEvent));
		}
		HitActors.Add(HitActor);
	}

	if (HitEvents.IsEmpty() || !ResolvedDamage.DamageSpec.IsValid() || !ResolvedDamage.DamageSpec.Data.IsValid()) return;

	UMASkillAbility* ExecutorAbility = nullptr;
	FMASkillScopes EventScopes;
	const FGameplayEffectContextHandle& ContextHandle = ResolvedDamage.DamageSpec.Data->GetContext();
	if (ResolveSkillEventSource(ContextHandle, ExecutorAbility, EventScopes))
	{
		// Instant multi-target hits are routed as one group so bindings execute once per attack.
		UMASkillEventRoutingStatics::TryNotifySkillEventGroup(ExecutorAbility, MoveTemp(HitEvents));
	}
}

void MADamageApplicator::ApplyToTargetActor(
	AActor& TargetActor,
	const FMAResolvedDamage& ResolvedDamage,
	const FVector& StatusEffectSourcePoint)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(&TargetActor);
	if (!TargetASC) return;

	const FVector TargetLocation = TargetActor.GetActorLocation();
	ApplyToTarget(
		*TargetASC,
		FHitResult(&TargetActor, nullptr, TargetLocation, FVector::UpVector),
		ResolvedDamage,
		StatusEffectSourcePoint);
}

void MADamageApplicator::ApplyToTarget(
	UAbilitySystemComponent& TargetASC,
	const FHitResult& HitResult,
	const FMAResolvedDamage& ResolvedDamage,
	const FVector& StatusEffectSourcePoint)
{
	FMASkillEvent HitEvent;
	if (!ApplyToTargetInternal(TargetASC, HitResult, ResolvedDamage, StatusEffectSourcePoint, HitEvent)) return;

	UMASkillAbility* ExecutorAbility = nullptr;
	FMASkillScopes EventScopes;
	const FGameplayEffectContextHandle& ContextHandle = ResolvedDamage.DamageSpec.Data->GetContext();
	if (ResolveSkillEventSource(ContextHandle, ExecutorAbility, EventScopes))
	{
		UMASkillEventRoutingStatics::TryNotifySkillEvent(ExecutorAbility, MoveTemp(HitEvent));
	}
}

bool MADamageApplicator::ApplyToTargetInternal(
	UAbilitySystemComponent& TargetASC,
	const FHitResult& HitResult,
	const FMAResolvedDamage& ResolvedDamage,
	const FVector& StatusEffectSourcePoint,
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
			ContextHandle);
		if (bHasDamageContext && ResolvedDamage.ApplicationMode == EMASkillDamageApplicationMode::Instant)
		{
			bHasHitEvent = PostProcessDamage(
				TargetASC,
				ContextHandle,
				HitResult,
				TargetASC.GetAvatarActor(),
				ResolvedDamage.TargetGameplayCueTags,
				OutHitEvent);
		}
	}

	ApplyStatusEffects(TargetASC, HitResult, ResolvedDamage, StatusEffectSourcePoint);
	return bHasHitEvent;
}

void MADamageApplicator::ApplyStatusEffects(
	UAbilitySystemComponent& TargetASC,
	const FHitResult& HitResult,
	const FMAResolvedDamage& ResolvedDamage,
	const FVector& StatusEffectSourcePoint)
{
	AActor* HitActor = HitResult.GetActor();
	for (const FResolvedStatusEffect& StatusEffect : ResolvedDamage.StatusEffects)
	{
		if (!ShouldApplyResolvedStatusEffect(TargetASC, HitActor, StatusEffect)) continue;

		FGameplayEffectSpecHandle SpecHandle = MakeSpecWithHitResult(HitResult, StatusEffect.SpecHandle);
		if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid()) continue;

		const FGameplayEffectContextHandle& ContextHandle = SpecHandle.Data->GetContext();
		UMAAbilitySystemStatics::SetReactionSourcePoint(
			SpecHandle,
			ResolveStatusEffectSourcePoint(ContextHandle, StatusEffectSourcePoint, StatusEffect.SourceType));
		const FActiveGameplayEffectHandle EffectHandle = TargetASC.ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		RegisterWithSkillRuntime(ContextHandle, TargetASC, EffectHandle);
	}
}
