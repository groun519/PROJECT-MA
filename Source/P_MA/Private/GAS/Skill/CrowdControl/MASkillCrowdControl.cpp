#include "GAS/Skill/CrowdControl/MASkillCrowdControl.h"

#include "GameplayEffect.h"
#include "GAS/MAAbilitySystemStatics.h"
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
}

bool UMASkillCrowdControl::BuildResolvedEffect(UMASkillAbility& SkillAbility, TArray<FResolvedCrowdControlEffect>& OutEffects) const
{
	FGameplayTag CrowdControlTag;
	float Magnitude = 0.f;
	float Duration = 0.f;
	EMASkillCrowdControlSourceType SourceType = EMASkillCrowdControlSourceType::Instigator;
	if (!ResolveSpecData(CrowdControlTag, Magnitude, Duration, SourceType)) return false;
	if (!CrowdControlTag.IsValid() || Duration <= 0.f) return false;

	FGameplayEffectSpecHandle SpecHandle = MakeTransientGameplayEffectSpec(SkillAbility, GetTransientDurationCrowdControlGameplayEffectTemplate(), 1.f);
	if (!SpecHandle.IsValid()) return false;

	SpecHandle.Data->DynamicGrantedTags.AddTag(CrowdControlTag);
	SpecHandle.Data->SetDuration(Duration, true);

	if (!FMath::IsNearlyZero(Magnitude))
	{
		SpecHandle.Data->SetSetByCallerMagnitude(CrowdControlTag, Magnitude);
	}

	ApplyCustomPayload(SpecHandle);

	FResolvedCrowdControlEffect ResolvedCrowdControlEffect;
	ResolvedCrowdControlEffect.SpecHandle = SpecHandle;
	ResolvedCrowdControlEffect.SourceType = SourceType;
	OutEffects.Add(ResolvedCrowdControlEffect);
	return true;
}

bool UMASkillCrowdControlStateBase::ResolveSpecData(
	FGameplayTag& OutCrowdControlTag,
	float& OutMagnitude,
	float& OutDuration,
	EMASkillCrowdControlSourceType& OutSourceType) const
{
	OutCrowdControlTag = GetCrowdControlTag();
	OutMagnitude = 0.f;
	OutDuration = Duration;
	OutSourceType = EMASkillCrowdControlSourceType::Instigator;
	return true;
}

bool UMASkillCrowdControlImpulseBase::ResolveSpecData(
	FGameplayTag& OutCrowdControlTag,
	float& OutMagnitude,
	float& OutDuration,
	EMASkillCrowdControlSourceType& OutSourceType) const
{
	OutCrowdControlTag = GetCrowdControlTag();
	OutMagnitude = Magnitude;
	OutDuration = Duration;
	OutSourceType = SourceType;
	return true;
}

FGameplayTag UMASkillCrowdControlStun::GetCrowdControlTag() const
{
	return UMAAbilitySystemStatics::GetStunStatTag();
}

FGameplayTag UMASkillCrowdControlRoot::GetCrowdControlTag() const
{
	return UMAAbilitySystemStatics::GetRootStatTag();
}

FGameplayTag UMASkillCrowdControlKnockback::GetCrowdControlTag() const
{
	return UMAAbilitySystemStatics::GetKnockbackStatTag();
}

FGameplayTag UMASkillCrowdControlGrab::GetCrowdControlTag() const
{
	return UMAAbilitySystemStatics::GetGrabStatTag();
}

FGameplayTag UMASkillCrowdControlStagger::GetCrowdControlTag() const
{
	return UMAAbilitySystemStatics::GetStaggerStatTag();
}

bool UMASkillCrowdControlAirborne::ResolveSpecData(
	FGameplayTag& OutCrowdControlTag,
	float& OutMagnitude,
	float& OutDuration,
	EMASkillCrowdControlSourceType& OutSourceType) const
{
	OutCrowdControlTag = UMAAbilitySystemStatics::GetAirborneStatTag();
	OutMagnitude = Magnitude;
	OutDuration = Duration;
	OutSourceType = EMASkillCrowdControlSourceType::Instigator;
	return true;
}

void UMASkillCrowdControlAirborne::ApplyCustomPayload(FGameplayEffectSpecHandle& SpecHandle) const
{
	if (SpecHandle.IsValid() && !FMath::IsNearlyZero(RiseTime))
	{
		SpecHandle.Data->SetSetByCallerMagnitude(UMAAbilitySystemStatics::GetAirborneRiseTimeTag(), RiseTime);
	}
}
