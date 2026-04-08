#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"

#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControlSpecBuilder.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Event/MASkillGameplayEventPart.h"
#include "GAS/Skill/Input/MASkillFlowPart.h"
#include "GAS/Skill/MASkillAbility.h"
#include "Animation/AnimInstance.h"
#include "GameplayEffect.h"
#include "GenericTeamAgentInterface.h"

void FSkillRuntimeContext::Initialize(UMASkillAbility* InOwnerAbility)
{
	OwnerAbility = InOwnerAbility;
	ResolvedDamageEffect = nullptr;
	ClearIgnoredActors();
	ClearDamageConfig();
}

void FSkillRuntimeContext::Reset()
{
	OwnerAbility = nullptr;
	ResolvedDamageEffect = nullptr;
	ClearIgnoredActors();
	ClearDamageConfig();
}

void FSkillRuntimeContext::HandleTagEvent(const FGameplayTag& EventTag)
{
	if (!EventTag.IsValid()) return;

	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	HandleEvent(Payload);
}

void FSkillRuntimeContext::HandleEvent(const FGameplayEventData& Payload)
{
	if (!OwnerAbility) return;

	RefreshStateFromEvent(Payload);

	TArray<UMASkillAction*> ResolvedActions;
	ResolveActionsForEvent(Payload, ResolvedActions);
	for (UMASkillAction* Action : ResolvedActions)
	{
		if (!Action) continue;
		Action->Execute(*this, Payload);
	}
}

void FSkillRuntimeContext::ClearDamageConfig()
{
	AccumulatedDamageConfig = FMASkillDamageConfig();
	AccumulatedTargetRelationModifiers.Reset();
}

void FSkillRuntimeContext::AddDamageConfig(const FMASkillDamageConfig& DamageConfig)
{
	AccumulatedDamageConfig.Append(DamageConfig);
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

	if (UMASkillFlowPart* RuntimeFlowPart = OwnerAbility->GetRuntimeFlowPart())
	{
		RuntimeFlowPart->CollectRequiredEventTags(RequiredTags);
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

bool FSkillRuntimeContext::HasAuthority() const
{
	return OwnerAbility && OwnerAbility->K2_HasAuthority();
}

AActor* FSkillRuntimeContext::GetAvatarActor() const
{
	return OwnerAbility ? OwnerAbility->GetAvatarActorFromActorInfo() : nullptr;
}

UAbilitySystemComponent* FSkillRuntimeContext::GetAbilitySystemComponent() const
{
	return OwnerAbility ? OwnerAbility->GetAbilitySystemComponentFromActorInfo() : nullptr;
}

USkeletalMeshComponent* FSkillRuntimeContext::GetOwningMeshComponent() const
{
	return OwnerAbility ? OwnerAbility->GetOwningComponentFromActorInfo() : nullptr;
}

UWorld* FSkillRuntimeContext::GetWorld() const
{
	return OwnerAbility ? OwnerAbility->GetWorld() : nullptr;
}

UAnimInstance* FSkillRuntimeContext::GetOwnerAnimInstance() const
{
	return OwnerAbility ? OwnerAbility->GetOwnerAnimInstance() : nullptr;
}

UAnimMontage* FSkillRuntimeContext::GetSkillMontage() const
{
	const UMASkillDefinition* SkillDefinition = OwnerAbility ? OwnerAbility->GetSkillDefinition() : nullptr;
	return SkillDefinition ? SkillDefinition->GetSkillMontage() : nullptr;
}

float FSkillRuntimeContext::GetAttributeValue(const FGameplayAttribute& Attribute, float DefaultValue) const
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
	if (!AbilitySystemComponent || !Attribute.IsValid())
	{
		return DefaultValue;
	}

	return AbilitySystemComponent->GetNumericAttribute(Attribute);
}

void FSkillRuntimeContext::SetDesiredMontagePlayRate(float PlayRate) const
{
	if (OwnerAbility)
	{
		OwnerAbility->SetDesiredMontagePlayRate(PlayRate);
	}
}

bool FSkillRuntimeContext::TryGetCurrentSkillSection(UAnimInstance*& OutAnimInstance, UAnimMontage*& OutSkillMontage, FName& OutCurrentSectionName) const
{
	OutAnimInstance = GetOwnerAnimInstance();
	OutSkillMontage = GetSkillMontage();
	if (!OutAnimInstance || !OutSkillMontage) return false;

	OutCurrentSectionName = OutAnimInstance->Montage_GetCurrentSection(OutSkillMontage);
	return !OutCurrentSectionName.IsNone();
}

TArray<FHitResult> FSkillRuntimeContext::GetHitResultsFromPayload(const FGameplayEventData& Payload, const FMASkillDamageConfig* DamageConfig) const
{
	if (!OwnerAbility) return TArray<FHitResult>();

	return OwnerAbility->GetHitResultFromVirtualSocketTargetData(Payload.TargetData, ResolveTargetRelationMask(DamageConfig));
}

FVector FSkillRuntimeContext::GetCrowdControlCenterPoint(const FGameplayEventData& Payload) const
{
	if (Payload.TargetData.Num() > 0 && Payload.TargetData.Data[0].IsValid())
	{
		return Payload.TargetData.Data[0]->GetOrigin().GetTranslation();
	}

	if (const AActor* AvatarActor = GetAvatarActor())
	{
		return AvatarActor->GetActorLocation();
	}

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
		ResolvedHitEffects.CrowdControlEffects = FMASkillCrowdControlSpecBuilder::BuildSpecs(*OwnerAbility, ResolvedDamageConfig);
	}
	return ResolvedHitEffects;
}

FGameplayEffectSpecHandle FSkillRuntimeContext::MakeDamageSpec(const FMASkillDamageConfig& ResolvedDamageConfig) const
{
	const TSubclassOf<UGameplayEffect> DamageEffect = ResolvedDamageEffect;
	if (!OwnerAbility || !DamageEffect) return FGameplayEffectSpecHandle();
	const FMADamageExecutionConfig ExecutionConfig = ResolvedDamageConfig.ToExecutionConfig();
	FGameplayEffectSpecHandle SpecHandle = OwnerAbility->MakeDamageEffectSpec(
		DamageEffect,
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
		if (const AActor* AvatarActor = GetAvatarActor())
		{
			return AvatarActor->GetActorLocation();
		}
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

		FGameplayEffectSpecHandle CrowdControlSpecHandle = CrowdControlEffect.SpecHandle;
		UMAAbilitySystemStatics::SetReactionSourcePoint(
			CrowdControlSpecHandle,
			ResolveCrowdControlSourcePoint(CrowdControlEffect.SourceType, CenterSourcePoint));
		OwnerAbility->ApplyGameplayEffectSpecToHitResultActor(HitResult, CrowdControlSpecHandle);
	}
}

FMASkillDamageConfig FSkillRuntimeContext::BuildMergedDamageConfig(const FMASkillDamageConfig* DamageConfig) const
{
	// TODO: Merge future runtime module damage contributions here before action-local config is appended.
	FMASkillDamageConfig Result = DamageConfig ? *DamageConfig : FMASkillDamageConfig();
	Result.Append(AccumulatedDamageConfig);
	return Result;
}

void FSkillRuntimeContext::RefreshStateFromEvent(const FGameplayEventData& Payload)
{
	const UMASkillDefinition* SkillDefinition = OwnerAbility ? OwnerAbility->GetSkillDefinition() : nullptr;
	ResolvedDamageEffect = SkillDefinition ? SkillDefinition->GetDefaultDamageEffect() : nullptr;

	if (UMASkillFlowPart* RuntimeFlowPart = OwnerAbility ? OwnerAbility->GetRuntimeFlowPart() : nullptr)
	{
		RuntimeFlowPart->HandleRuntimeEvent(Payload);
	}

	// TODO: Apply additional definition-derived state refresh here before module contributions are merged.
}
