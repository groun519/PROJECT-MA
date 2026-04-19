#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"

#include "AbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControl.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/MAGameplayEffect_SkillDamage.h"
#include "GameplayEffect.h"
#include "AbilitySystemBlueprintLibrary.h"

void FSkillRuntimeContext::Initialize(UMASkillAbility* InOwnerAbility)
{
	OwnerAbility = InOwnerAbility;
	ClearIgnoredActors();
	ClearDamageConfig();
}

void FSkillRuntimeContext::Reset()
{
	OwnerAbility = nullptr;
	ClearIgnoredActors();
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

TArray<FHitResult> FSkillRuntimeContext::GetHitResultsFromPayload(const FGameplayEventData& Payload, int32 TargetRelationMask) const
{
	if (!OwnerAbility) return TArray<FHitResult>();

	return OwnerAbility->GetHitResultFromVirtualSocketTargetData(Payload.TargetData, TargetRelationMask);
}

FVector FSkillRuntimeContext::GetCrowdControlCenterPoint(const FGameplayEventData& Payload) const
{
	if (Payload.TargetData.Num() > 0 && Payload.TargetData.Data[0].IsValid()) return Payload.TargetData.Data[0]->GetOrigin().GetTranslation();

	if (const AActor* AvatarActor = OwnerAbility ? OwnerAbility->GetAvatarActorFromActorInfo() : nullptr) return AvatarActor->GetActorLocation();

	return FVector::ZeroVector;
}

int32 FSkillRuntimeContext::ResolveTargetRelationMask(int32 BaseRelationMask) const
{
	int32 ResolvedRelationMask = BaseRelationMask;

	for (const FMASkillTargetRelationModifier& TargetRelationModifier : AccumulatedTargetRelationModifiers)
	{
		TargetRelationModifier.ApplyTo(ResolvedRelationMask);
	}

	return ResolvedRelationMask;
}

FResolvedSkillHitEffects FSkillRuntimeContext::BuildResolvedHitEffects(const FMASkillDamageConfig& BaseDamageConfig) const
{
	FResolvedSkillHitEffects ResolvedHitEffects;
	const FMASkillDamageConfig ResolvedDamageConfig = BuildMergedDamageConfig(BaseDamageConfig);

	ResolvedHitEffects.TargetRelationMask = ResolveTargetRelationMask(BaseDamageConfig.TargetRelationMask);
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

FMASkillDamageConfig FSkillRuntimeContext::BuildMergedDamageConfig(const FMASkillDamageConfig& BaseDamageConfig) const
{
	FMASkillDamageConfig Result = BaseDamageConfig;
	Result.Append(AccumulatedDamageConfig);
	Result.FinalDamageMultiplier *= AccumulatedFinalDamageMultiplier;
	return Result;
}
