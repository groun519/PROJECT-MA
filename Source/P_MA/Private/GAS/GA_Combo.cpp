// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_Combo.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"
#include "MASkillVFXSet.h"
#include "Character/MACharacter.h"
#include "GAS/MAAbilitySystemStatics.h"

UGA_Combo::UGA_Combo()
{
	AbilityTags.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
	BlockAbilitiesWithTag.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
	ActivationBlockedTags.AddTag(UMAAbilitySystemStatics::GetAimingTag());
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Debuff"));

	VFXRootTag = FGameplayTag::RequestGameplayTag("Event.VFX");
}

void UGA_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	IgnoreTargets.Empty();
	
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayComboMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ComboMontage);
		PlayComboMontageTask->OnBlendOut.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnCancelled.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnCompleted.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnInterrupted.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitComboChangeEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetComboChangeEventTag(), nullptr, false, false);
		WaitComboChangeEventTask->EventReceived.AddDynamic(this, &UGA_Combo::ComboChangedEventReceived);
		WaitComboChangeEventTask->ReadyForActivation();
	}

	if (K2_HasAuthority())
	{
		UAbilityTask_WaitGameplayEvent* WaitTargetEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetComboTargetEventTag());
		WaitTargetEventTask->EventReceived.AddDynamic(this, &UGA_Combo::DoDamage);
		WaitTargetEventTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitClearEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetComboClearEventTag());
		WaitClearEventTask->EventReceived.AddDynamic(this, &UGA_Combo::ClearIgnore);
		WaitClearEventTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitVFXEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, VFXRootTag, nullptr,false,false);
		WaitVFXEventTask->EventReceived.AddDynamic(this, &UGA_Combo::HandleVFXSpawnEvent);
		WaitVFXEventTask->ReadyForActivation();
	}
	SetupWaitComboInputPress();
}

FGameplayTag UGA_Combo::GetComboChangeEventTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Combo.Change");
}

FGameplayTag UGA_Combo::GetComboChangeEventEndTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Combo.Change.End");
}

FGameplayTag UGA_Combo::GetComboTargetEventTag()
{
	//return FGameplayTag::RequestGameplayTag("Ability.Combo.Damage");
	return FGameplayTag::RequestGameplayTag("Event.Montage.Damage");
}

FGameplayTag UGA_Combo::GetComboClearEventTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Combo.Clear");
}

void UGA_Combo::SetupWaitComboInputPress()
{
	UAbilityTask_WaitInputPress* WaitInputPress = UAbilityTask_WaitInputPress::WaitInputPress(this);
	WaitInputPress->OnPress.AddDynamic(this, &UGA_Combo::HandleInputPress);
	WaitInputPress->ReadyForActivation();
}

void UGA_Combo::HandleInputPress(float TimeWaited)
{
	SetupWaitComboInputPress();
	TryCommitCombo();
}

void UGA_Combo::TryCommitCombo()
{
	if (NextComboName == NAME_None)
	{
		return;
	}

	UAnimInstance* OwnerAnimInst = GetOwnerAnimInstance();
	if (!OwnerAnimInst)
	{
		return;
	}

	OwnerAnimInst->Montage_SetNextSection(OwnerAnimInst->Montage_GetCurrentSection(ComboMontage), NextComboName, ComboMontage);
}

TSubclassOf<UGameplayEffect> UGA_Combo::GetDamageEffectForCurrentCombo() const
{
	UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance();
	if (OwnerAnimInstance)
	{
		FName CurrentSectionName = OwnerAnimInstance->Montage_GetCurrentSection(ComboMontage);
		const TSubclassOf<UGameplayEffect>* FoundEffectPtr = DamageEffectMap.Find(CurrentSectionName);
		if (FoundEffectPtr)
		{
			return *FoundEffectPtr;
		}
	}

	return DefaultDamageEffect;
}

void UGA_Combo::ComboChangedEventReceived(FGameplayEventData Data)
{
	FGameplayTag EventTag = Data.EventTag;
	
	if (EventTag == GetComboChangeEventEndTag())
	{
		NextComboName = NAME_None;
		return;
	}

	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);
	NextComboName = TagNames.Last();
}

void UGA_Combo::DoDamage(FGameplayEventData Data)
{
	TArray<FHitResult> HitResults =
		GetHitResultFromVirtualSocketTargetData(Data.TargetData);

	for (const FHitResult& HitResult : HitResults)
	{
		if (IgnoreTargets.Contains(HitResult.GetActor())) continue;
			
		TSubclassOf<UGameplayEffect> GameplayEffect = GetDamageEffectForCurrentCombo();
		ApplyGameplayEffectToHitResultActor(HitResult, GameplayEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		IgnoreTargets.Add(HitResult.GetActor());
	}

	if (FuryEffect && !IgnoreTargets.IsEmpty())
	{
		FGameplayEffectSpecHandle FuryEffectSpec = MakeOutgoingGameplayEffectSpec(FuryEffect, GetAbilityLevel(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo()));
		ApplyGameplayEffectSpecToOwner(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), CurrentActivationInfo, FuryEffectSpec);

		IgnoreTargets.Empty();
	}
}

void UGA_Combo::ClearIgnore(FGameplayEventData Data)
{
	
	FGameplayTag ClearTag = Data.EventTag;

	if (ClearTag == GetComboClearEventTag())
	{
		IgnoreTargets.Empty();
	}
}

void UGA_Combo::HandleVFXSpawnEvent(FGameplayEventData Payload)
{
	if (!HasAuthority(&CurrentActivationInfo))
		return;
	if (!VFXDataSet)
		return;

	const F_SkillVFX_Info* VFXInfo = VFXDataSet->VFXDataMap.Find(Payload.EventTag);
	if (!VFXInfo || !VFXInfo->DefaultVFX)
		return;
	
	FLinearColor SpawnColor = FLinearColor::White;
	bool bApplyColor = false;

	AMACharacter* Character = Cast<AMACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
		return;
	USkeletalMeshComponent* MeshComp = Character->GetMesh();
	if (!MeshComp)
		return;

	if (VFXInfo->bSpawnInWorld)
	{
		FTransform SocketTransform = (VFXInfo->SocketName != NAME_None)? MeshComp->GetSocketTransform(VFXInfo->SocketName) : MeshComp->GetComponentTransform();
		FTransform OffsetTransform(VFXInfo->RotationOffset, VFXInfo->LocationOffset, VFXInfo->Scale);
		FTransform WorldSPawnTransform = OffsetTransform * SocketTransform;

		Character->Multicast_PlayNiagara(VFXInfo->DefaultVFX, WorldSPawnTransform, bApplyColor, SpawnColor);
	}
	else
	{
		Character->Multicast_PlayNiagaraAttached(VFXInfo->DefaultVFX,VFXInfo->SocketName,VFXInfo->LocationOffset,VFXInfo->RotationOffset,VFXInfo->Scale,	VFXInfo->bAutoDestroy,bApplyColor, SpawnColor);
	}
}
