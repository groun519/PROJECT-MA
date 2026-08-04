#include "GAS/Skill/MASkillAbility.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "Character/MACharacter.h"
#include "Character/MAImpulseComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/PA_AbilitySystemGenerics.h"
#include "GAS/Skill/Addon/MASkillModuleAddonStatics.h"
#include "GAS/Skill/Addon/Sequence/MASkillModuleSequenceAddon.h"
#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/MASkillSystemTypes.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Sequence/MASkillSequenceRuntime.h"
#include "Setting/MAGameSettings.h"

UMASkillAbility::UMASkillAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	SequenceRuntime = CreateDefaultSubobject<UMASkillSequenceRuntime>(TEXT("SequenceRuntime"));

	const FGameplayTag SkillTag = FGameplayTag::RequestGameplayTag(TEXT("Skill"));
	AbilityTags.AddTag(SkillTag);
	BlockAbilitiesWithTag.AddTag(SkillTag);

	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetStunStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetFrozenStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetAirborneStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetGrabStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetStaggerStatTag());
}

void UMASkillAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (const AMACharacter* OwnerCharacter = Cast<AMACharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr))
	{
		if (UMASkillManagerComponent* SkillManager = OwnerCharacter->GetSkillManagerComponent())
		{
			SkillManager->RegisterAbilityHandle(FMASkillSystemStatics::ResolveSlotTagFromAbilitySpec(Spec), Spec.Handle, GetClass());
		}
	}
}

void UMASkillAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);

	if (const AMACharacter* OwnerCharacter = Cast<AMACharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr))
	{
		if (UMASkillManagerComponent* SkillManager = OwnerCharacter->GetSkillManagerComponent())
		{
			SkillManager->UnregisterAbilityHandle(FMASkillSystemStatics::ResolveSlotTagFromAbilitySpec(Spec), Spec.Handle);
		}
	}

	UpdateCurrentSkillModuleInstance(nullptr);
}

const UMASkillModule* UMASkillAbility::GetCurrentSkillModule() const
{
	return CurrentSkillModuleInstance ? CurrentSkillModuleInstance->GetRootModule() : nullptr;
}

FMASkillPayloadStore* UMASkillAbility::GetModulePayloadStore(UMASkillModuleInstance* BindingScope) const
{
	return BindingScope ? &BindingScope->GetPayloadStore() : nullptr;
}

FMASkillPayloadStore& UMASkillAbility::GetAssembledModulePayloadStore()
{
	check(CurrentSkillModuleInstance);
	return CurrentSkillModuleInstance->GetPayloadStore();
}

void UMASkillAbility::UpdateCurrentSkillModuleInstance(UMASkillModuleInstance* SourceSkillModuleInstance)
{
	check(!IsActive());

	CurrentSkillModuleInstance = SourceSkillModuleInstance;
	if (!CurrentSkillModuleInstance)
	{
		SequenceRuntime->ResetSequence();
		return;
	}

	const UMASkillModuleSequenceAddon* SequenceAddon =
		MASkillModuleAddonStatics::FindAddon<UMASkillModuleSequenceAddon>(*CurrentSkillModuleInstance);
	if (!SequenceAddon)
	{
		SequenceRuntime->ResetSequence();
		return;
	}

	SequenceRuntime->UpdateSequence(SequenceAddon->GetSequences());
}

void UMASkillAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	const UMASkillModule* CurrentSkillModule = GetCurrentSkillModule();
	if (!CurrentSkillModuleInstance || !CurrentSkillModule) { K2_EndAbility(); return; }
	if (!K2_CommitAbility()) { K2_EndAbility(); return; }

	CurrentSkillModuleInstance->ResetPayloadStore();
	CurrentSkillModule->ApplyPayloadsTo(CurrentSkillModuleInstance->GetPayloadStore());

	SequenceRuntime->SetDesiredPlayRate(1.f);
	RegisterCancelTriggers();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	SkillActivatedDelegate.Broadcast();
	if (CurrentSkillModuleInstance)
	{
		UMASkillEventRoutingStatics::TryNotifySkillEvent(this, FMASkillEvent(
			FGameplayTag::RequestGameplayTag(TEXT("Event.Skill.Activate")),
			FMASkillScopes{ nullptr, CurrentSkillModuleInstance }));
	}
	if (IsActive())
	{
		SequenceRuntime->Start();
	}
}

bool UMASkillAbility::CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCooldown(Handle, ActorInfo, OptionalRelevantTags)) return false;

	const float CooldownSeconds = GetCooldownSeconds();
	if (CooldownSeconds <= 0.f) return true;

	const FGameplayTag CooldownTag = GetCooldownTagForSpec(Handle, ActorInfo);
	if (!CooldownTag.IsValid()) return true;

	UAbilitySystemComponent* AbilitySystemComponent = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!AbilitySystemComponent || !AbilitySystemComponent->HasMatchingGameplayTag(CooldownTag)) return true;

	if (OptionalRelevantTags)
	{
		OptionalRelevantTags->AddTag(CooldownTag);
	}
	return false;
}

void UMASkillAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);

	const float CooldownSeconds = GetCooldownSeconds();
	if (CooldownSeconds <= 0.f) return;

	const FGameplayTag CooldownTag = GetCooldownTagForSpec(Handle, ActorInfo);
	if (!CooldownTag.IsValid()) return;

	const UPA_AbilitySystemGenerics* SystemGenerics = UMAGameSettings::Get()->GetAbilitySystemGenerics();
	TSubclassOf<UGameplayEffect> CooldownEffect = SystemGenerics ? SystemGenerics->GetCooldownEffect() : nullptr;
	if (!ensureMsgf(CooldownEffect, TEXT("Cooldown effect is not configured for ASC owner %s."),
		*GetNameSafe(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr)))
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownEffect, GetAbilityLevel());
	if (!SpecHandle.IsValid()) return;

	SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.Cooldown.Duration")), CooldownSeconds);
	SpecHandle.Data->DynamicGrantedTags.AddTag(CooldownTag);
	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
}

void UMASkillAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	SequenceRuntime->Stop();
	SkillDeactivatedDelegate.Broadcast();
	if (CurrentSkillModuleInstance)
	{
		UMASkillEventRoutingStatics::TryNotifySkillEvent(this, FMASkillEvent(
			FGameplayTag::RequestGameplayTag(TEXT("Event.Skill.End")),
			FMASkillScopes{ nullptr, CurrentSkillModuleInstance }));
	}

	UnregisterCancelTriggers();
	if (AMACharacter* OwnerCharacter = Cast<AMACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UMASkillManagerComponent* SkillManager = OwnerCharacter->GetSkillManagerComponent())
		{
			SkillManager->ClearActivePreviewVisualElementTag();
		}

		if (bWasCancelled)
		{
			if (UMAImpulseComponent* ImpulseComponent = OwnerCharacter->GetImpulseComponent())
			{
				ImpulseComponent->StopOwnedActionImpulses(this);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

float UMASkillAbility::GetCooldownSeconds() const
{
	const UMASkillModule* CurrentSkillModule = GetCurrentSkillModule();
	return CurrentSkillModule ? CurrentSkillModule->GetCooldownSeconds() : 0.f;
}

FGameplayTag UMASkillAbility::GetCooldownTagForSpec(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	UAbilitySystemComponent* AbilitySystemComponent = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent ? AbilitySystemComponent->FindAbilitySpecFromHandle(Handle) : nullptr;
	return AbilitySpec
		? FMASkillSystemStatics::ResolveCooldownTagFromSlotTag(FMASkillSystemStatics::ResolveSlotTagFromAbilitySpec(*AbilitySpec))
		: FGameplayTag();
}

FGameplayTag UMASkillAbility::GetVisualElementTag() const
{
	static const FGameplayTag DefaultVisualElementTag = UMAAbilitySystemStatics::GetDefaultVisualElementTag();
	const UMASkillModule* CurrentSkillModule = GetCurrentSkillModule();
	if (!CurrentSkillModule) return DefaultVisualElementTag;

	const FGameplayTag VisualElementTag = CurrentSkillModule->GetVisualElementTag();
	return VisualElementTag.IsValid()
		? VisualElementTag
		: DefaultVisualElementTag;
}

UMASkillModuleInstance* UMASkillAbility::GetCurrentBindingScope() const
{
	const FMASkillScopes* CurrentScopes = SequenceRuntime->GetCurrentTargetScopes();
	return CurrentScopes ? CurrentScopes->Module.Get() : nullptr;
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

UMASkillManagerComponent* UMASkillAbility::GetSkillManagerComponent() const
{
	const AMACharacter* OwnerCharacter = Cast<AMACharacter>(GetAvatarActorFromActorInfo());
	return OwnerCharacter ? OwnerCharacter->GetSkillManagerComponent() : nullptr;
}
