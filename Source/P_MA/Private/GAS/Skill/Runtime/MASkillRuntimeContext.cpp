#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Projectile/MAProjectile.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Event/MASkillGameplayEventPart.h"
#include "GAS/Skill/Input/MASkillFlowPart.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GameFramework/Pawn.h"
#include "Animation/AnimInstance.h"

void FSkillRuntimeContext::Initialize(UMASkillAbility* InOwnerAbility)
{
	OwnerAbility = InOwnerAbility;
	ResolvedDamageEffect = nullptr;
	ClearIgnoredActors();
	ClearDamageConfig();
}

void FSkillRuntimeContext::Reset()
{
	ResolvedDamageEffect = nullptr;
	ClearIgnoredActors();
	ClearDamageConfig();
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
	AccumulatedDamageConfig = FMADamageExecutionConfig();
}

void FSkillRuntimeContext::AddDamageConfig(const FMADamageExecutionConfig& DamageConfig)
{
	AccumulatedDamageConfig.Append(DamageConfig);
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

bool FSkillRuntimeContext::TryGetCurrentSkillSection(UAnimInstance*& OutAnimInstance, UAnimMontage*& OutSkillMontage, FName& OutCurrentSectionName) const
{
	OutAnimInstance = GetOwnerAnimInstance();
	OutSkillMontage = GetSkillMontage();
	if (!OutAnimInstance || !OutSkillMontage) return false;

	OutCurrentSectionName = OutAnimInstance->Montage_GetCurrentSection(OutSkillMontage);
	return !OutCurrentSectionName.IsNone();
}

TArray<FHitResult> FSkillRuntimeContext::GetHitResultsFromPayload(const FGameplayEventData& Payload) const
{
	return OwnerAbility ? OwnerAbility->GetHitResultFromVirtualSocketTargetData(Payload.TargetData) : TArray<FHitResult>();
}

FGameplayEffectSpecHandle FSkillRuntimeContext::MakeDamageSpec(const FMADamageExecutionConfig* DamageConfig) const
{
	const TSubclassOf<UGameplayEffect> DamageEffect = ResolvedDamageEffect;
	if (!OwnerAbility || !DamageEffect) return FGameplayEffectSpecHandle();

	const FMADamageExecutionConfig ResolvedDamageConfig = BuildMergedDamageConfig(DamageConfig);
	return OwnerAbility->MakeDamageEffectSpec(DamageEffect, 1, ResolvedDamageConfig.HasValues() ? &ResolvedDamageConfig : nullptr);
}

void FSkillRuntimeContext::ApplyDamageToHitResult(const FHitResult& HitResult, const FMADamageExecutionConfig* DamageConfig) const
{
	const TSubclassOf<UGameplayEffect> DamageEffect = ResolvedDamageEffect;
	if (!OwnerAbility || !DamageEffect) return;

	const FMADamageExecutionConfig ResolvedDamageConfig = BuildMergedDamageConfig(DamageConfig);
	OwnerAbility->ApplyGameplayEffectToHitResultActor(
		HitResult,
		DamageEffect,
		1,
		ResolvedDamageConfig.HasValues() ? &ResolvedDamageConfig : nullptr);
}

AMAProjectile* FSkillRuntimeContext::SpawnDamageProjectile(
	TSubclassOf<AMAProjectile> ProjectileClass,
	const FVector& SpawnLocation,
	const FRotator& SpawnRotation,
	const FMADamageExecutionConfig* DamageConfig,
	float ExplodeRadius,
	bool bIsPenetrating) const
{
	if (!OwnerAbility || !ProjectileClass) return nullptr;

	UWorld* World = GetWorld();
	AActor* AvatarActor = GetAvatarActor();
	if (!World || !AvatarActor) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = AvatarActor;
	SpawnParams.Instigator = Cast<APawn>(SpawnParams.Owner);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMAProjectile* Projectile = World->SpawnActor<AMAProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (!Projectile) return nullptr;

	Projectile->InitializeProjectile(MakeDamageSpec(DamageConfig), ExplodeRadius, bIsPenetrating);
	return Projectile;
}

FMADamageExecutionConfig FSkillRuntimeContext::BuildMergedDamageConfig(const FMADamageExecutionConfig* DamageConfig) const
{
	// TODO: Merge future runtime module damage contributions here before action-local config is appended.
	FMADamageExecutionConfig Result = AccumulatedDamageConfig;
	if (DamageConfig)
	{
		Result.Append(*DamageConfig);
	}
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
