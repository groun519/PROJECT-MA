#include "GAS/Skill/CrowdControl/MASkillCrowdControlSpecBuilder.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControlSpecBuilderInternal.h"

#include "GameplayEffect.h"
#include "GAS/Skill/MASkillAbility.h"
#include "UObject/StrongObjectPtr.h"

namespace
{
UGameplayEffect* GetTransientDurationCrowdControlGameplayEffectTemplate()
{
	static TStrongObjectPtr<UGameplayEffect> DurationEffectTemplate;

	if (!DurationEffectTemplate.IsValid())
	{
		UGameplayEffect* NewTemplate = NewObject<UGameplayEffect>(GetTransientPackage(), NAME_None, RF_Transient);
		NewTemplate->DurationPolicy = EGameplayEffectDurationType::HasDuration;
		DurationEffectTemplate.Reset(NewTemplate);
	}

	return DurationEffectTemplate.Get();
}

FGameplayEffectSpecHandle MakeTransientGameplayEffectSpec(UMASkillAbility& SkillAbility, const UGameplayEffect* EffectDefinition, float Level)
{
	if (!EffectDefinition) return FGameplayEffectSpecHandle();

	FGameplayEffectSpecHandle SpecHandle(new FGameplayEffectSpec(
		EffectDefinition,
		SkillAbility.MakeEffectContext(SkillAbility.GetCurrentAbilitySpecHandle(), SkillAbility.GetCurrentActorInfo()),
		Level));

	if (SpecHandle.IsValid())
	{
		if (FGameplayAbilitySpec* AbilitySpec = SkillAbility.GetCurrentAbilitySpec())
		{
			SkillAbility.ApplyAbilityTagsToGameplayEffectSpec(*SpecHandle.Data.Get(), AbilitySpec);
			SpecHandle.Data->SetByCallerTagMagnitudes = AbilitySpec->SetByCallerTagMagnitudes;
		}
	}

	return SpecHandle;
}

const TMap<const UScriptStruct*, MASkillCrowdControlSpecBuilderInternal::FCrowdControlSpecBuildHandler>& GetCrowdControlSpecBuildHandlers()
{
	using namespace MASkillCrowdControlSpecBuilderInternal;

	static const TMap<const UScriptStruct*, FCrowdControlSpecBuildHandler> Handlers =
	{
		{ FSkillCrowdControlStunConfig::StaticStruct(), &TryBuildStunCrowdControlSpec },
		{ FSkillCrowdControlAirborneConfig::StaticStruct(), &TryBuildAirborneCrowdControlSpec },
		{ FSkillCrowdControlKnockbackConfig::StaticStruct(), &TryBuildKnockbackCrowdControlSpec },
		{ FSkillCrowdControlGrabConfig::StaticStruct(), &TryBuildGrabCrowdControlSpec },
		{ FSkillCrowdControlStaggerConfig::StaticStruct(), &TryBuildStaggerCrowdControlSpec },
	};

	return Handlers;
}
}

bool MASkillCrowdControlSpecBuilderInternal::BuildCrowdControlEntry(
	const FGameplayTag CrowdControlTag,
	const float Magnitude,
	const float Duration,
	const EMASkillCrowdControlSourceType SourceType,
	FMASkillCrowdControlEntry& OutEntry)
{
	OutEntry.CrowdControlTag = CrowdControlTag;
	OutEntry.Magnitude = Magnitude;
	OutEntry.Duration = Duration;
	OutEntry.SourceType = SourceType;
	return OutEntry.HasValidData();
}

FGameplayEffectSpecHandle MASkillCrowdControlSpecBuilderInternal::MakeResolvedCrowdControlSpec(UMASkillAbility& SkillAbility, const FMASkillCrowdControlEntry& Entry)
{
	if (!Entry.HasValidData()) return FGameplayEffectSpecHandle();

	FGameplayEffectSpecHandle SpecHandle = MakeTransientGameplayEffectSpec(SkillAbility, GetTransientDurationCrowdControlGameplayEffectTemplate(), 1.f);
	if (!SpecHandle.IsValid()) return SpecHandle;

	SpecHandle.Data->DynamicGrantedTags.AddTag(Entry.CrowdControlTag);
	SpecHandle.Data->SetDuration(Entry.Duration, true);

	if (!FMath::IsNearlyZero(Entry.Magnitude))
	{
		SpecHandle.Data->SetSetByCallerMagnitude(Entry.CrowdControlTag, Entry.Magnitude);
	}

	return SpecHandle;
}

void MASkillCrowdControlSpecBuilderInternal::AddResolvedCrowdControlEffect(
	const FGameplayEffectSpecHandle& SpecHandle,
	const EMASkillCrowdControlSourceType SourceType,
	TArray<FResolvedCrowdControlEffect>& OutEffects)
{
	if (!SpecHandle.IsValid()) return;

	FResolvedCrowdControlEffect ResolvedCrowdControlEffect;
	ResolvedCrowdControlEffect.SpecHandle = SpecHandle;
	ResolvedCrowdControlEffect.SourceType = SourceType;
	OutEffects.Add(ResolvedCrowdControlEffect);
}

TArray<FResolvedCrowdControlEffect> FMASkillCrowdControlSpecBuilder::BuildSpecs(UMASkillAbility& SkillAbility, const FMASkillDamageConfig& DamageConfig)
{
	TArray<FResolvedCrowdControlEffect> CrowdControlEffectSpecs;

	for (const FInstancedStruct& CrowdControlConfig : DamageConfig.CrowdControlConfigs)
	{
		const UScriptStruct* CrowdControlStruct = CrowdControlConfig.GetScriptStruct();
		if (!CrowdControlStruct) continue;

		if (const MASkillCrowdControlSpecBuilderInternal::FCrowdControlSpecBuildHandler* Handler = GetCrowdControlSpecBuildHandlers().Find(CrowdControlStruct))
		{
			(*Handler)(CrowdControlConfig, SkillAbility, CrowdControlEffectSpecs);
		}
	}

	return CrowdControlEffectSpecs;
}
