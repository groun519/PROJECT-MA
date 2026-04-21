#include "GAS/Skill/Action/MASkillAction_MeleeOverlapHelper.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Engine/HitResult.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GameplayEffect.h"
#include "GAS/Skill/MASkillDamageConfig.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"

namespace MASkillActionMeleeOverlap
{
	namespace
	{
		FVector ResolveCrowdControlSourcePoint(
			UMASkillAbility& OwnerAbility,
			EMASkillCrowdControlSourceType SourceType,
			const FVector& CenterSourcePoint)
		{
			switch (SourceType)
			{
			case EMASkillCrowdControlSourceType::Center:
				return CenterSourcePoint;
			case EMASkillCrowdControlSourceType::Instigator:
			default:
				if (const AActor* AvatarActor = OwnerAbility.GetAvatarActorFromActorInfo()) return AvatarActor->GetActorLocation();
				return CenterSourcePoint;
			}
		}

		bool ShouldApplyResolvedCrowdControlEffect(
			AActor* TargetActor,
			const FResolvedCrowdControlEffect& CrowdControlEffect)
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
	}

	FMASkillDamageConfig ResolveDamageConfig(const FMASkillPayloadStore& PayloadStore, const FGameplayTag& DamagePayloadTag)
	{
		FMASkillDamageConfig DamageConfig;
		PayloadStore.TryGetStruct(DamagePayloadTag, DamageConfig);
		return DamageConfig;
	}

	TArray<FHitResult> ResolveHitResultsFromPayload(
		UMASkillAbility& OwnerAbility,
		const FGameplayEventData& Payload,
		int32 TargetRelationMask)
	{
		return OwnerAbility.GetHitResultFromVirtualSocketTargetData(Payload.TargetData, TargetRelationMask);
	}

	FVector ResolveCrowdControlCenterPoint(
		UMASkillAbility& OwnerAbility,
		const FGameplayEventData& Payload)
	{
		if (Payload.TargetData.Num() > 0 && Payload.TargetData.Data[0].IsValid())
		{
			return Payload.TargetData.Data[0]->GetOrigin().GetTranslation();
		}

		if (const AActor* AvatarActor = OwnerAbility.GetAvatarActorFromActorInfo())
		{
			return AvatarActor->GetActorLocation();
		}

		return FVector::ZeroVector;
	}

	void ApplyHitResults(
		UMASkillAbility& OwnerAbility,
		TSet<TWeakObjectPtr<AActor>>& IgnoredActors,
		const TArray<FHitResult>& HitResults,
		const FResolvedSkillHitEffects& ResolvedHitEffects,
		const FVector& CrowdControlCenterPoint)
	{
		TSet<AActor*> HitActors;
		for (const FHitResult& HitResult : HitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			if (!HitActor || HitActors.Contains(HitActor) || IgnoredActors.Contains(HitActor))
			{
				continue;
			}

			if (ResolvedHitEffects.DamageSpec.IsValid())
			{
				OwnerAbility.ApplyGameplayEffectSpecToHitResultActor(HitResult, ResolvedHitEffects.DamageSpec);
			}

			for (const FResolvedCrowdControlEffect& CrowdControlEffect : ResolvedHitEffects.CrowdControlEffects)
			{
				if (!CrowdControlEffect.SpecHandle.IsValid()) continue;
				if (!ShouldApplyResolvedCrowdControlEffect(HitActor, CrowdControlEffect)) continue;

				FGameplayEffectSpecHandle CrowdControlSpecHandle = CrowdControlEffect.SpecHandle;
				UMAAbilitySystemStatics::SetReactionSourcePoint(
					CrowdControlSpecHandle,
					ResolveCrowdControlSourcePoint(OwnerAbility, CrowdControlEffect.SourceType, CrowdControlCenterPoint));
				OwnerAbility.ApplyGameplayEffectSpecToHitResultActor(HitResult, CrowdControlSpecHandle);
			}

			HitActors.Add(HitActor);
			IgnoredActors.Add(HitActor);
		}
	}
}
