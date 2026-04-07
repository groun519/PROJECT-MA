// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Modules/SkillModule_Combo.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Ability/MAGameplayAbility_Skill.h"

void USkillModule_Combo::OnAbilityActivated()
{
	bComboInputPressed = false;
	bIsComboActive = false;

	if (OwnerSkill)
	{
		FGameplayEventData Payload;
		Payload.EventTag = FGameplayTag::RequestGameplayTag("UI.Skill.ComboIconReady");
		Payload.OptionalObject = OwnerSkill;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerSkill->GetAvatarActorFromActorInfo(), Payload.EventTag, Payload);
		
		InputPressTask = UAbilityTask_WaitInputPress::WaitInputPress(OwnerSkill);
		InputPressTask->OnPress.AddDynamic(this, &USkillModule_Combo::OnComboInputPressed);
		InputPressTask->ReadyForActivation();
	}
}

void USkillModule_Combo::OnAbilityEnded(bool bWasCancelled)
{
	if (InputPressTask)
		InputPressTask->EndTask();
	if (InputWindowTask)
		InputWindowTask->EndTask();
	if (ComboMontageTask)
		ComboMontageTask->EndTask();
	if (DamageEventTask)
		DamageEventTask->EndTask();
}

void USkillModule_Combo::ModifyDamageSpec(FGameplayEffectSpecHandle& SpecHandle) const
{
	Super::ModifyDamageSpec(SpecHandle);
}

void USkillModule_Combo::CreateAdditionalEffectSpecs(TArray<FGameplayEffectSpecHandle>& OutAdditionalSpecs) const
{
	if (bIsComboActive && OwnerSkill)
	{
		const FBehavior_Combo* ComboConfig = OwnerSkill->GetComboData().ModuleConfig.GetPtr<FBehavior_Combo>();
		if (ComboConfig && ComboConfig->ComboModuleEffect)
		{
			FGameplayEffectSpecHandle AddSpec = OwnerSkill->MakeOutgoingGameplayEffectSpec(ComboConfig->ComboModuleEffect);
			if (AddSpec.IsValid())
			{
				OutAdditionalSpecs.Add(AddSpec);
			}
		}
	}
}

bool USkillModule_Combo::TryActivateCombo()
{
	if (!OwnerSkill)
		return false;

	const FModuleBehaviorData& ComboData = OwnerSkill->GetComboData();
	const FBehavior_Combo* ComboConfig = ComboData.ModuleConfig.GetPtr<FBehavior_Combo>();
	if (!ComboConfig || !ComboConfig->ComboMontage)
		return false;

	if (bComboInputPressed)
	{
		StartComboMontage();
		return true;
	}

	if (ComboConfig->InputWindow > 0.f)
	{
		AActor* AvatarActor = OwnerSkill->GetAvatarActorFromActorInfo();
		
		FGameplayEventData Payload;
		Payload.EventTag = FGameplayTag::RequestGameplayTag("UI.Skill.ComboWaitStart");
		Payload.EventMagnitude = ComboConfig->InputWindow;
		Payload.OptionalObject = OwnerSkill; 
		
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(AvatarActor, Payload.EventTag, Payload);
		
		InputWindowTask = UAbilityTask_WaitDelay::WaitDelay(OwnerSkill, ComboConfig->InputWindow);
		InputWindowTask->OnFinish.AddDynamic(this, &USkillModule_Combo::OnInputWindowEnded);
		InputWindowTask->ReadyForActivation();
		return true;
	}
	return false;
}

void USkillModule_Combo::OnComboInputPressed(float TimeWaited)
{
	bComboInputPressed = true;

	if (InputWindowTask)
	{
		InputWindowTask->EndTask();
		InputWindowTask = nullptr;
		StartComboMontage();
	}
}

void USkillModule_Combo::StartComboMontage()
{
	if (!OwnerSkill)
		return;

	FGameplayEventData EndPayload;
	EndPayload.EventTag = FGameplayTag::RequestGameplayTag("UI.Skill.ComboWaitEnd");
	EndPayload.OptionalObject = OwnerSkill;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerSkill->GetAvatarActorFromActorInfo(), EndPayload.EventTag, EndPayload);

	bIsComboActive = true;

	const FModuleBehaviorData& ComboData = OwnerSkill->GetComboData();
	const FBehavior_Combo* ComboConfig = ComboData.ModuleConfig.GetPtr<FBehavior_Combo>();

	float PlayRate = OwnerSkill->GetTotalAnimSpeed();
	ComboMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(OwnerSkill, NAME_None, ComboConfig->ComboMontage,PlayRate);
	ComboMontageTask->OnCompleted.AddDynamic(this, &USkillModule_Combo::OnComboMontageEnded);
	ComboMontageTask->OnInterrupted.AddDynamic(this, &USkillModule_Combo::OnComboMontageEnded);
	ComboMontageTask->OnBlendOut.AddDynamic(this, &USkillModule_Combo::OnComboMontageEnded);
	ComboMontageTask->OnCancelled.AddDynamic(this, &USkillModule_Combo::OnComboMontageEnded);
	ComboMontageTask->ReadyForActivation();

	DamageEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwnerSkill, UMAAbilitySystemStatics::GetMontageDamageTag());
	DamageEventTask->EventReceived.AddDynamic(this, &USkillModule_Combo::OnComboDamageEvent);
	DamageEventTask->ReadyForActivation();
}

void USkillModule_Combo::OnComboMontageEnded()
{
	if (OwnerSkill)
		OwnerSkill->EndAbility(OwnerSkill->GetCurrentAbilitySpecHandle(), OwnerSkill->GetCurrentActorInfo(), OwnerSkill->GetCurrentActivationInfo(), true, false);
}

void USkillModule_Combo::OnComboDamageEvent(FGameplayEventData Payload)
{
	if (OwnerSkill)
		OwnerSkill->ExecuteSkillAction(Payload, 1.f);
}

void USkillModule_Combo::OnInputWindowEnded()
{
	if (OwnerSkill)
	{
		FGameplayEventData Payload;
		Payload.EventTag = FGameplayTag::RequestGameplayTag("UI.Skill.ComboWaitEnd");
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerSkill->GetAvatarActorFromActorInfo(), Payload.EventTag, Payload);
		
		OwnerSkill->EndAbility(OwnerSkill->GetCurrentAbilitySpecHandle(), OwnerSkill->GetCurrentActorInfo(), OwnerSkill->GetCurrentActivationInfo(), true, false);
	}
}
