#include "GAS/MAGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Skill/Area/MASkillAreaStatics.h"
#include "GAS/Skill/Area/MASkillAreaTypes.h"

UMAGameplayAbility::UMAGameplayAbility()
{
	ActivationBlockedTags.AddTag(UMAAbilitySystemStatics::GetAbilityBlockTag());
	ActivationBlockedTags.AddTag(UMAAbilitySystemStatics::GetInputBlockTag());
}

UAnimInstance* UMAGameplayAbility::GetOwnerAnimInstance() const
{
	USkeletalMeshComponent* OwnerSkeletalMeshComp = GetOwningComponentFromActorInfo();
	if (OwnerSkeletalMeshComp) return OwnerSkeletalMeshComp->GetAnimInstance();
	return nullptr;
}

TArray<FHitResult> UMAGameplayAbility::GetHitResultsFromAreaTargetData(
	const FGameplayAbilityTargetDataHandle& Handle)
{
	return GetHitResultsFromAreaTargetData(Handle, MATargetRelation::GetDefaultMask());
}

TArray<FHitResult> UMAGameplayAbility::GetHitResultsFromAreaTargetData(
	const FGameplayAbilityTargetDataHandle& Handle,
	int32 OverrideTargetRelationMask)
{
	const FMASkillWorldAreaShape* Area = MASkillAreaStatics::FindWorldShape(Handle);
	UWorld* World = GetWorld();
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!Area || !World || !AvatarActor) return {};

	if (Area->bDrawDebug) MASkillAreaStatics::DrawWorldPreview(*World, *Area);
	return MASkillAreaStatics::ResolveHitResults(*World, AvatarActor, *Area, OverrideTargetRelationMask);
}

void UMAGameplayAbility::StopMontageAfterCurrentSection(UAnimMontage* Montage)
{
	UAnimInstance* OwnerAnimInst = GetOwnerAnimInstance();
	if (OwnerAnimInst)
	{
		FName CurrentSectionName = OwnerAnimInst->Montage_GetCurrentSection(Montage);
		OwnerAnimInst->Montage_SetNextSection(CurrentSectionName, NAME_None, Montage);
	}
}

void UMAGameplayAbility::PlayMontageLocally(UAnimMontage* Montage)
{
	UAnimInstance* OwnerAnimIst = GetOwnerAnimInstance();
	if (OwnerAnimIst && !OwnerAnimIst->Montage_IsPlaying(Montage))
	{
		OwnerAnimIst->Montage_Play(Montage);
	}
}

ACharacter* UMAGameplayAbility::GetOwningAvatarCharacter()
{
	if (!AvatarCharacter)
	{
		AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	}
	return AvatarCharacter;
}

FGenericTeamId UMAGameplayAbility::GetOwnerTeamId() const
{
	IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());
	if (OwnerTeamInterface)
	{
		return OwnerTeamInterface->GetGenericTeamId();
	}
	return FGenericTeamId::NoTeam;
}

void UMAGameplayAbility::ApplyGameplayEffectToHitResultActor(const FHitResult& HitResult,
                                                             TSubclassOf<UGameplayEffect> GameplayEffect, int Level,
                                                             const FMADamageExecutionConfig* DamageConfig)
{
	FGameplayEffectSpecHandle EffectSpecHandle = MakeDamageEffectSpec(GameplayEffect, Level, DamageConfig);
	if (!EffectSpecHandle.IsValid()) return;

	ApplyGameplayEffectSpecToHitResultActor(HitResult, EffectSpecHandle);
}

void UMAGameplayAbility::ApplyGameplayEffectSpecToHitResultActor(const FHitResult& HitResult,
                                                                 const FGameplayEffectSpecHandle& EffectSpecHandle)
{
	if (!EffectSpecHandle.IsValid()) return;

	FGameplayEffectContextHandle EffectContext = EffectSpecHandle.Data->GetContext().Duplicate();
	EffectContext.AddHitResult(HitResult);

	EffectSpecHandle.Data->SetContext(EffectContext);

	ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(), CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor()));
}

FGameplayEffectSpecHandle UMAGameplayAbility::MakeDamageEffectSpec(
	TSubclassOf<UGameplayEffect> GameplayEffect,
	int32 Level,
	const FMADamageExecutionConfig* DamageConfig)
{
	FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(GameplayEffect, Level);
	if (DamageConfig)
	{
		UMAAbilitySystemStatics::ApplyDamageExecutionConfig(EffectSpecHandle, *DamageConfig);
	}
	return EffectSpecHandle;
}
