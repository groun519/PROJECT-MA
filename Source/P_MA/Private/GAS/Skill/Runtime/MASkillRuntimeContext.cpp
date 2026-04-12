#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"

#include "AbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControl.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Event/MASkillGameplayEventPart.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/MAGameplayEffect_SkillDamage.h"
#include "GameplayEffect.h"
#include "AbilitySystemBlueprintLibrary.h"

void FSkillRuntimeContext::Initialize(UMASkillAbility* InOwnerAbility)
{
	OwnerAbility = InOwnerAbility;
	ClearIgnoredActors();
	ClearPayload();
	ClearDamageConfig();
}

void FSkillRuntimeContext::Reset()
{
	OwnerAbility = nullptr;
	ClearIgnoredActors();
	ClearPayload();
	ClearDamageConfig();
}

void FSkillRuntimeContext::ClearDamageConfig()
{
	AccumulatedDamageConfig = FMASkillDamageConfig();
	AccumulatedFinalDamageMultiplier = 1.f;
	AccumulatedTargetRelationModifiers.Reset();
}

void FSkillRuntimeContext::AddDamageConfig(const FMASkillDamageConfig& DamageConfig)
{
	AccumulatedDamageConfig.Append(DamageConfig);
}

void FSkillRuntimeContext::MultiplyFinalDamageMultiplier(float Multiplier)
{
	AccumulatedFinalDamageMultiplier *= Multiplier;
}

void FSkillRuntimeContext::AddTargetRelationModifier(const FMASkillTargetRelationModifier& TargetRelationModifier)
{
	if (!TargetRelationModifier.HasOverride()) return;
	AccumulatedTargetRelationModifiers.Add(TargetRelationModifier);
}

TSet<FGameplayTag> FSkillRuntimeContext::ResolveRequiredEventTags() const
{
	TSet<FGameplayTag> RequiredTags;

	const UMASkillDefinition* SkillDefinition = OwnerAbility ? OwnerAbility->GetSkillDefinition() : nullptr;
	if (!SkillDefinition) return RequiredTags;

	for (const FMASkillGameplayEventPart& EventPart : SkillDefinition->GetEventParts())
	{
		if (!EventPart.EventTag.IsValid() || !EventPart.Action) continue;
		RequiredTags.Add(EventPart.EventTag);
	}

	// TODO: Merge additional required event tags from runtime modules here.
	return RequiredTags;
}

void FSkillRuntimeContext::ResolveActionsForEvent(const FGameplayEventData& Payload, TArray<UMASkillAction*>& OutActions) const
{
	OutActions.Reset();

	const UMASkillDefinition* SkillDefinition = OwnerAbility ? OwnerAbility->GetSkillDefinition() : nullptr;
	if (!SkillDefinition) return;

	for (const FMASkillGameplayEventPart& EventPart : SkillDefinition->GetEventParts())
	{
		if (EventPart.EventTag != Payload.EventTag || !EventPart.Action) continue;
		OutActions.Add(EventPart.Action);
	}

	// TODO: Append runtime module action contributions for this event here.
}

TArray<FHitResult> FSkillRuntimeContext::GetHitResultsFromPayload(const FGameplayEventData& Payload, const FMASkillDamageConfig* DamageConfig) const
{
	if (!OwnerAbility) return TArray<FHitResult>();

	return OwnerAbility->GetHitResultFromVirtualSocketTargetData(Payload.TargetData, ResolveTargetRelationMask(DamageConfig));
}

FVector FSkillRuntimeContext::GetCrowdControlCenterPoint(const FGameplayEventData& Payload) const
{
	if (Payload.TargetData.Num() > 0 && Payload.TargetData.Data[0].IsValid()) return Payload.TargetData.Data[0]->GetOrigin().GetTranslation();

	if (const AActor* AvatarActor = OwnerAbility ? OwnerAbility->GetAvatarActorFromActorInfo() : nullptr) return AvatarActor->GetActorLocation();

	return FVector::ZeroVector;
}

int32 FSkillRuntimeContext::ResolveTargetRelationMask(const FMASkillDamageConfig* DamageConfig) const
{
	int32 ResolvedRelationMask = DamageConfig
		? DamageConfig->TargetRelationMask
		: MATargetRelation::ToMask(EMATargetRelation::None);

	for (const FMASkillTargetRelationModifier& TargetRelationModifier : AccumulatedTargetRelationModifiers)
	{
		TargetRelationModifier.ApplyTo(ResolvedRelationMask);
	}

	return ResolvedRelationMask;
}

FResolvedSkillHitEffects FSkillRuntimeContext::BuildResolvedHitEffects(const FMASkillDamageConfig* DamageConfig) const
{
	FResolvedSkillHitEffects ResolvedHitEffects;
	const FMASkillDamageConfig ResolvedDamageConfig = BuildMergedDamageConfig(DamageConfig);

	ResolvedHitEffects.TargetRelationMask = ResolveTargetRelationMask(DamageConfig);
	ResolvedHitEffects.DamageSpec = MakeDamageSpec(ResolvedDamageConfig);
	if (OwnerAbility)
	{
		for (const TObjectPtr<UMASkillCrowdControl>& CrowdControl : ResolvedDamageConfig.CrowdControls)
		{
			if (!CrowdControl) continue;
			CrowdControl->BuildResolvedEffect(*OwnerAbility, ResolvedHitEffects.CrowdControlEffects);
		}
	}
	return ResolvedHitEffects;
}

FGameplayEffectSpecHandle FSkillRuntimeContext::MakeDamageSpec(const FMASkillDamageConfig& ResolvedDamageConfig) const
{
	if (!OwnerAbility) return FGameplayEffectSpecHandle();
	const FMADamageExecutionConfig ExecutionConfig = ResolvedDamageConfig.ToExecutionConfig();
	FGameplayEffectSpecHandle SpecHandle = OwnerAbility->MakeDamageEffectSpec(
		UMAGameplayEffect_SkillDamage::StaticClass(),
		1,
		ExecutionConfig.HasValues() ? &ExecutionConfig : nullptr);

	return SpecHandle;
}

FVector FSkillRuntimeContext::ResolveCrowdControlSourcePoint(EMASkillCrowdControlSourceType SourceType, const FVector& CenterSourcePoint) const
{
	switch (SourceType)
	{
	case EMASkillCrowdControlSourceType::Center:
		return CenterSourcePoint;
	case EMASkillCrowdControlSourceType::Instigator:
	default:
		if (const AActor* AvatarActor = OwnerAbility ? OwnerAbility->GetAvatarActorFromActorInfo() : nullptr) return AvatarActor->GetActorLocation();
		return CenterSourcePoint;
	}
}

void FSkillRuntimeContext::ApplyResolvedHitEffectsToHitResult(const FHitResult& HitResult, const FResolvedSkillHitEffects& ResolvedHitEffects, const FVector& CenterSourcePoint) const
{
	if (!OwnerAbility) return;

	if (ResolvedHitEffects.DamageSpec.IsValid())
	{
		OwnerAbility->ApplyGameplayEffectSpecToHitResultActor(HitResult, ResolvedHitEffects.DamageSpec);
	}

	for (const FResolvedCrowdControlEffect& CrowdControlEffect : ResolvedHitEffects.CrowdControlEffects)
	{
		if (!CrowdControlEffect.SpecHandle.IsValid()) continue;
		if (!ShouldApplyResolvedCrowdControlEffect(HitResult.GetActor(), CrowdControlEffect)) continue;

		FGameplayEffectSpecHandle CrowdControlSpecHandle = CrowdControlEffect.SpecHandle;
		UMAAbilitySystemStatics::SetReactionSourcePoint(
			CrowdControlSpecHandle,
			ResolveCrowdControlSourcePoint(CrowdControlEffect.SourceType, CenterSourcePoint));
		OwnerAbility->ApplyGameplayEffectSpecToHitResultActor(HitResult, CrowdControlSpecHandle);
	}
}

bool FSkillRuntimeContext::ShouldApplyResolvedCrowdControlEffect(
	AActor* TargetActor,
	const FResolvedCrowdControlEffect& CrowdControlEffect) const
{
	if (CrowdControlEffect.StrengthPolicy == EMASkillStatusEffectStrengthPolicy::None) return true;
	if (!TargetActor || !CrowdControlEffect.SpecHandle.IsValid() || !CrowdControlEffect.SpecHandle.Data.IsValid()) return true;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return true;

	const UGameplayEffect* EffectDefinition = CrowdControlEffect.SpecHandle.Data->Def.Get();
	if (!EffectDefinition) return true;

	const UClass* EffectDefinitionClass = EffectDefinition->GetClass();
	FGameplayEffectQuery Query(FActiveGameplayEffectQueryCustomMatch::CreateLambda(
		[EffectDefinitionClass](const FActiveGameplayEffect& ActiveEffect)
		{
			const UGameplayEffect* ActiveDefinition = ActiveEffect.Spec.Def.Get();
			return ActiveDefinition && ActiveDefinition->GetClass() == EffectDefinitionClass;
		}));

	const TArray<FActiveGameplayEffectHandle> ActiveEffects = TargetASC->GetActiveEffects(Query);
	if (ActiveEffects.Num() == 0) return true;

	bool bApplyIncomingEffect = true;
	for (const FActiveGameplayEffectHandle ActiveEffectHandle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveEffect = TargetASC->GetActiveGameplayEffect(ActiveEffectHandle);
		if (!ActiveEffect) continue;

		const float ExistingStrengthMagnitude = ActiveEffect->Spec.GetSetByCallerMagnitude(
			FGameplayTag::RequestGameplayTag(TEXT("Data.StatusEffect.StrengthMagnitude")),
			false,
			CrowdControlEffect.StrengthMagnitude);

		switch (CrowdControlEffect.StrengthPolicy)
		{
		case EMASkillStatusEffectStrengthPolicy::LargerMagnitudeStronger:
			if (ExistingStrengthMagnitude > CrowdControlEffect.StrengthMagnitude
				&& !FMath::IsNearlyEqual(ExistingStrengthMagnitude, CrowdControlEffect.StrengthMagnitude))
			{
				bApplyIncomingEffect = false;
			}
			break;
		case EMASkillStatusEffectStrengthPolicy::SmallerMagnitudeStronger:
			if (ExistingStrengthMagnitude < CrowdControlEffect.StrengthMagnitude
				&& !FMath::IsNearlyEqual(ExistingStrengthMagnitude, CrowdControlEffect.StrengthMagnitude))
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
		TargetASC->RemoveActiveGameplayEffect(ActiveEffectHandle);
	}

	return true;
}

FMASkillDamageConfig FSkillRuntimeContext::BuildMergedDamageConfig(const FMASkillDamageConfig* DamageConfig) const
{
	// TODO: Merge future runtime module damage contributions here before action-local config is appended.
	FMASkillDamageConfig Result = DamageConfig ? *DamageConfig : FMASkillDamageConfig();
	Result.Append(AccumulatedDamageConfig);
	Result.FinalDamageMultiplier *= AccumulatedFinalDamageMultiplier;
	return Result;
}

void FSkillRuntimeContext::AddPayload(const FGameplayTag& Tag)
{
	if (Tag.IsValid())
	{
		PayloadTags.AddTag(Tag);
	}
}

bool FSkillRuntimeContext::HasPayload(const FGameplayTag& Tag) const
{
	if (!Tag.IsValid()) return false;

	return PayloadTags.HasTagExact(Tag)
		|| PayloadScalars.Contains(Tag)
		|| PayloadVectors.Contains(Tag)
		|| PayloadObjects.Contains(Tag);
}

void FSkillRuntimeContext::SetPayload(const FGameplayTag& Key, float Value)
{
	if (Key.IsValid())
	{
		PayloadScalars.FindOrAdd(Key) = Value;
	}
}

bool FSkillRuntimeContext::TryGetPayload(const FGameplayTag& Key, float& OutValue) const
{
	if (!Key.IsValid()) return false;

	if (const float* Value = PayloadScalars.Find(Key))
	{
		OutValue = *Value;
		return true;
	}

	return false;
}

void FSkillRuntimeContext::SetPayload(const FGameplayTag& Key, const FVector& Value)
{
	if (Key.IsValid())
	{
		PayloadVectors.FindOrAdd(Key) = Value;
	}
}

bool FSkillRuntimeContext::TryGetPayload(const FGameplayTag& Key, FVector& OutValue) const
{
	if (!Key.IsValid()) return false;

	if (const FVector* Value = PayloadVectors.Find(Key))
	{
		OutValue = *Value;
		return true;
	}

	return false;
}

void FSkillRuntimeContext::SetPayload(const FGameplayTag& Key, UObject* Value)
{
	if (Key.IsValid())
	{
		PayloadObjects.FindOrAdd(Key) = Value;
	}
}

bool FSkillRuntimeContext::TryGetPayload(const FGameplayTag& Key, UObject*& OutValue) const
{
	if (!Key.IsValid()) return false;

	if (const TObjectPtr<UObject>* Value = PayloadObjects.Find(Key))
	{
		OutValue = Value->Get();
		return true;
	}

	return false;
}

void FSkillRuntimeContext::ClearPayload()
{
	PayloadTags.Reset();
	PayloadScalars.Reset();
	PayloadVectors.Reset();
	PayloadObjects.Reset();
}

void FSkillRuntimeContext::RefreshStateFromEvent(const FGameplayEventData& Payload)
{
	(void)Payload;
	// TODO: Apply additional definition-derived state refresh here before module contributions are merged.
}
