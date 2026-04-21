#include "GAS/Skill/MASkillAbility.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "Character/MACharacter.h"
#include "Character/MAImpulseComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"

UMASkillAbility::UMASkillAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetStunStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetAirborneStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetGrabStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetStaggerStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetKnockbackStatTag());
}

void UMASkillAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	CacheRuntimeSkillDefinition(Handle, ActorInfo);
	if (!GetSkillDefinition()) { K2_EndAbility(); return; }
	if (!K2_CommitAbility()) { K2_EndAbility(); return; }

	PayloadStore.Reset();
	GetSkillDefinition()->ApplyPayloadsTo(PayloadStore);
	RuntimeContext.Initialize(this);
	DesiredMontagePlayRate = 1.f;
	RuntimeSkillDefinition->ActivateSkill(this);
	RegisterCancelTriggers();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UMASkillAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (RuntimeSkillDefinition)
	{
		RuntimeSkillDefinition->DeactivateSkill();
	}

	UnregisterCancelTriggers();
	if (AMACharacter* OwnerCharacter = Cast<AMACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UMAImpulseComponent* ImpulseComponent = OwnerCharacter->GetImpulseComponent())
		{
			ImpulseComponent->StopOwnedActionImpulses(this);
		}
	}

	RuntimeContext.Reset();
	PayloadStore.Reset();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

const UMASkillDefinition* UMASkillAbility::GetSkillDefinition() const
{
	return RuntimeSkillDefinition ? RuntimeSkillDefinition : SkillDefinition;
}

void UMASkillAbility::HandleSkillTagEvent(const FGameplayTag& EventTag)
{
	if (!RuntimeSkillDefinition) return;

	RuntimeSkillDefinition->HandleSkillTagEvent(EventTag, RuntimeContext, PayloadStore);
}

const FGameplayTag& UMASkillAbility::GetElementalTag() const
{
	static const FGameplayTag EmptyTag;
	return GetSkillDefinition() ? GetSkillDefinition()->GetElementalTag() : EmptyTag;
}

void UMASkillAbility::SetDesiredMontagePlayRate(float NewPlayRate)
{
	DesiredMontagePlayRate = FMath::Max(NewPlayRate, 0.f);

	if (RuntimeSkillDefinition)
	{
		RuntimeSkillDefinition->ApplyDesiredMontagePlayRate(DesiredMontagePlayRate);
	}
}

bool UMASkillAbility::GetSkillProgressInfo(FText& OutLabel, float& OutDuration, float& OutRemainingDuration) const
{
	const UMASkillDefinition* RuntimeDefinition = GetSkillDefinition();
	return RuntimeDefinition && RuntimeDefinition->GetSkillProgressInfo(OutLabel, OutDuration, OutRemainingDuration);
}

bool UMASkillAbility::CanPlaySkillMontageLocally() const
{
	const FGameplayAbilityActorInfo* ActorInfo = CurrentActorInfo;
	return ActorInfo
		&& (HasAuthorityOrPredictionKey(ActorInfo, &CurrentActivationInfo) || ActorInfo->IsLocallyControlled());
}

void UMASkillAbility::RegisterCancelTriggers()
{
	UnregisterCancelTriggers();

	if (CancelTriggerTags.IsEmpty()) return;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	for (const FGameplayTag& CancelTriggerTag : CancelTriggerTags)
	{
		if (!CancelTriggerTag.IsValid()) continue;

		FDelegateHandle DelegateHandle = ASC->RegisterGameplayTagEvent(CancelTriggerTag).AddUObject(this, &UMASkillAbility::HandleCancelTriggerTagChanged);
		CancelTriggerDelegateHandles.Add(CancelTriggerTag, DelegateHandle);
	}
}

void UMASkillAbility::UnregisterCancelTriggers()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		CancelTriggerDelegateHandles.Reset();
		return;
	}

	for (const TPair<FGameplayTag, FDelegateHandle>& DelegatePair : CancelTriggerDelegateHandles)
	{
		ASC->RegisterGameplayTagEvent(DelegatePair.Key).Remove(DelegatePair.Value);
	}

	CancelTriggerDelegateHandles.Reset();
}

void UMASkillAbility::HandleCancelTriggerTagChanged(FGameplayTag Tag, int32 NewCount)
{
	if (NewCount != 1) return;
	if (!IsActive()) return;

	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}

void UMASkillAbility::CacheRuntimeSkillDefinition(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo)
{
	if (RuntimeSkillDefinition) return;

	const UMASkillDefinition* ResolvedSkillDefinition = SkillDefinition;
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		const FGameplayAbilitySpec* AbilitySpec = ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
		if (AbilitySpec)
		{
			if (const UMASkillDefinition* SourceDefinition = Cast<UMASkillDefinition>(AbilitySpec->SourceObject.Get()))
			{
				ResolvedSkillDefinition = SourceDefinition;
			}
		}
	}

	if (!ResolvedSkillDefinition) return;
	RuntimeSkillDefinition = DuplicateObject<UMASkillDefinition>(const_cast<UMASkillDefinition*>(ResolvedSkillDefinition), this);
}
