// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/WeaponEffectInterface.h"
#include "Player/MAPlayerCharacter.h"

UMAGameplayAbility_SkillBase::UMAGameplayAbility_SkillBase()
{
	DamageEventTag = UMAAbilitySystemStatics::GetMontageDamageTag();
	AttributeCueTag = UMAAbilitySystemStatics::GetSkillAttributeTag();
}

void UMAGameplayAbility_SkillBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	bIsEnd = false;
	bIsHoldEnd = false;

	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,SkillAnimMontage);
	PlayMontageTask->OnBlendOut.AddDynamic(this, &UMAGameplayAbility_SkillBase::K2_EndAbility);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UMAGameplayAbility_SkillBase::K2_EndAbility);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &UMAGameplayAbility_SkillBase::K2_EndAbility);
	PlayMontageTask->OnCompleted.AddDynamic(this, &UMAGameplayAbility_SkillBase::K2_EndAbility);
	PlayMontageTask->ReadyForActivation();
	
	// --- Module 1) Utility ---
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		for (const TSubclassOf<UGameplayEffect>& UtilityEffect : ModuleUtility)
		{
			if (UtilityEffect)
			{
				FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(UtilityEffect);
				if (SpecHandle.IsValid())
					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}

	// Module 2) Attribute
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	IWeaponEffectInterface* WeaponEffect = Cast<IWeaponEffectInterface>(AvatarActor);
	if (WeaponEffect && AttributeEffects && ModuleAttributeTag.IsValid())
	{
		UNiagaraSystem* EffectToPlay = AttributeEffects->EffectMap.FindRef(ModuleAttributeTag);
		if (EffectToPlay)
			WeaponEffect->ActivateWeaponEffect(EffectToPlay);
	}

	/* 멀티 플레이어에서 변경되도록 Cue 사용은 작동 안함 - 내가 잘 모르나봄. K2_ExecuteGameplayCueWithParams() 써도 안되고 별 지랄..
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
	   if (ASC->GetOwnerRole() == ROLE_Authority)
	   {
		  FGameplayCueParameters CueParams;
		  CueParams.MatchedTagName = ModuleAttributeTag;
		  ASC->ExecuteGameplayCue(AttributeCueTag, CueParams);
	   }
	}
	*/

	// Module 3) Behavior
	const FGameplayTagContainer& DynamicTags = GetCurrentAbilitySpec()->DynamicAbilityTags;
	bool bIsChargingSkill = DynamicTags.HasTag(UMAAbilitySystemStatics::GetChargeSkillTag());
	bool bIsChainSkill = DynamicTags.HasTag(UMAAbilitySystemStatics::GetChainSkillTag());
	bool bIsHoldingSkill = DynamicTags.HasTag(UMAAbilitySystemStatics::GetHoldSkillTag());
	
	if (bIsChargingSkill)
	{
		UE_LOG(LogTemp, Warning, TEXT("Behavior: Charge (From Dynamic Tag)"));
		HandleChargeSkill();
	}
	else if (bIsChainSkill)
	{
		UE_LOG(LogTemp, Warning, TEXT("Behavior: Chain (From Dynamic Tag)"));
		HandleChainSkill();
	}
	else if (bIsHoldingSkill)
	{
		UE_LOG(LogTemp, Warning, TEXT("Behavior: Hold (From Dynamic Tag)"));
		HandleHoldingSkill();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Behavior: Default"));
		HandleDefaultSkill();
	}
}

void UMAGameplayAbility_SkillBase::HandleDefaultSkill()
{
	UE_LOG(LogTemp, Warning, TEXT("Default"));
	
}

void UMAGameplayAbility_SkillBase::HandleChargeSkill()
{	//최대 차지 시간
	UAbilityTask_WaitDelay* ChargeTimeout = UAbilityTask_WaitDelay::WaitDelay(this, MaxChargeDuration);
	ChargeTimeout->OnFinish.AddDynamic(this, &UMAGameplayAbility_SkillBase::OnMaxCharged);
	ChargeTimeout->ReadyForActivation();
	//애니메이션 느리게
	UAbilityTask_WaitGameplayEvent* WaitSlowTagTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Event.Montage.SlowPlay"));
	WaitSlowTagTask->EventReceived.AddDynamic(this, &UMAGameplayAbility_SkillBase::OnChargeEventReceived);
	WaitSlowTagTask->ReadyForActivation();
	//차지 중 키 놓으면
	UAbilityTask_WaitInputRelease* InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	InputReleaseTask->OnRelease.AddDynamic(this, &UMAGameplayAbility_SkillBase::OnChargeReleased);
	InputReleaseTask->ReadyForActivation();
}

void UMAGameplayAbility_SkillBase::HandleHoldingSkill()
{	//최대 홀딩 시간
	UAbilityTask_WaitDelay* HoldTimeOut = UAbilityTask_WaitDelay::WaitDelay(this, MaxHoldDuration);
	HoldTimeOut->OnFinish.AddDynamic(this, &UMAGameplayAbility_SkillBase::OnMaxHold);
	HoldTimeOut->ReadyForActivation();
	
	//애니메이션 거꾸로 재생하도록
	UAbilityTask_WaitGameplayEvent* WaitReverseTagTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,FGameplayTag::RequestGameplayTag("Event.Montage.ReversePlay"));
	WaitReverseTagTask->EventReceived.AddDynamic(this, &UMAGameplayAbility_SkillBase::OnReversePlay);
	WaitReverseTagTask->ReadyForActivation();

	//홀딩 중 키 놓으면
	UAbilityTask_WaitInputRelease* InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	InputReleaseTask->OnRelease.AddDynamic(this, &UMAGameplayAbility_SkillBase::OnHoldReleased);
	InputReleaseTask->ReadyForActivation();
}
void UMAGameplayAbility_SkillBase::HandleChainSkill()
{
	UE_LOG(LogTemp, Warning, TEXT("Skill chains"));
	K2_EndAbility();
}

void UMAGameplayAbility_SkillBase::OnChargeEventReceived(FGameplayEventData EventData)
{
	SetMontagePlayRate(0.01f);
}
void UMAGameplayAbility_SkillBase::OnChargeReleased(float Time)
{
	if (bIsEnd)
		return;
	bIsEnd = true;
	SetMontagePlayRate(1.f);
}
void UMAGameplayAbility_SkillBase::OnMaxCharged()
{
	if (bIsEnd)
		return;
	bIsEnd = true;
	SetMontagePlayRate(1.f);
}


void UMAGameplayAbility_SkillBase::OnForwardPlay(FGameplayEventData EventData)
{
	if (bIsHoldEnd)	return;
	
	SetMontagePlayRate(1.f);
	UAbilityTask_WaitGameplayEvent* WaitReverseTagTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,FGameplayTag::RequestGameplayTag("Event.Montage.ReversePlay"));
	WaitReverseTagTask->EventReceived.AddDynamic(this, &UMAGameplayAbility_SkillBase::OnReversePlay);
	WaitReverseTagTask->ReadyForActivation();
}

void UMAGameplayAbility_SkillBase::OnReversePlay(FGameplayEventData EventData)
{
	if (bIsHoldEnd)	return;
	
	SetMontagePlayRate(ReverseSpeed);
	UAbilityTask_WaitGameplayEvent* WaitForwardTagTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,FGameplayTag::RequestGameplayTag("Event.Montage.ForwardPlay"));
	WaitForwardTagTask->EventReceived.AddDynamic(this, &UMAGameplayAbility_SkillBase::OnForwardPlay);
	WaitForwardTagTask->ReadyForActivation();
}
void UMAGameplayAbility_SkillBase::OnMaxHold()
{
	if (bIsEnd)
		return;
	bIsEnd = true;
	bIsHoldEnd = true;
	SetMontagePlayRate(1.f);
	MontageToOtherSection(EndSection);
}
void UMAGameplayAbility_SkillBase::OnHoldReleased(float Time)
{
	if (bIsEnd)
		return;
	bIsEnd = true;
	bIsHoldEnd = true;
	SetMontagePlayRate(1.f);
	MontageToOtherSection(EndSection);
	K2_EndAbility();
}



void UMAGameplayAbility_SkillBase::SetMontagePlayRate(float NewPlayRate)
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
		{
			UAnimMontage* ActiveMontage  = AnimInstance->GetCurrentActiveMontage();
			AnimInstance->Montage_SetPlayRate(ActiveMontage ,NewPlayRate);
		}
	}
}

void UMAGameplayAbility_SkillBase::MontageToOtherSection(FName SectionName)
{
	AMAPlayerCharacter* Character = Cast<AMAPlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
	if (Character)
	{
		if (SkillAnimMontage)
		{
			Character->GetMesh()->GetAnimInstance()->Montage_JumpToSection(SectionName,SkillAnimMontage);
		}
	}
}
