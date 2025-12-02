// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/MASkillBehavior.h"

#include "GameplayTagsManager.h"
#include "GAS/MASkillVFXSet.h"
#include "MAGameplayAbility_SkillBase.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "SkillBehaviorConfig.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Engine/DataTable.h"
#include "GameFramework/PlayerController.h"
#include "Player/MAPlayerCharacter.h"

void UMASkillBehavior::OnActivate_Implementation()
{
	OwningAbility->IgnoreTargets.Empty();
	this->Character = GetCharacter();
	this->PlayerCharacter = Cast<AMAPlayerCharacter>(this->Character);

	FGameplayTag RootTag = OwningAbility->GetVFXRootTag();
	if (OwningAbility && RootTag.IsValid())
	{
		WaitVFXEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility,RootTag,nullptr,false,false);
		WaitVFXEventTask->EventReceived.AddDynamic(this, &UMASkillBehavior::HandleVFXSpawnEvent);
		WaitVFXEventTask->ReadyForActivation();
	}
	if (OwningAbility)
	{
		MontageToPlay = OwningAbility->GetSkillMontage();
	}
}

void UMASkillBehavior::OnEndAbility_Implementation()
{
	if (WaitVFXEventTask.IsValid())
		WaitVFXEventTask->EndTask();
	
	this->Character = nullptr;
	this->PlayerCharacter = nullptr;
}

float UMASkillBehavior::GetCurrentDamageMultiplier() const
{
	return BehaviorDamageMultiplier;
}

void UMASkillBehavior::InitFromConfig(const FInstancedStruct& ConfigPayload)
{
	const FSkillBehaviorConfigBase* BaseConfig = ConfigPayload.GetPtr<FSkillBehaviorConfigBase>();
	if (BaseConfig)
	{
		CooldownDuration = BaseConfig->CooldownDuration;
		DamageMultiplier = BaseConfig->DamageMultiplier;
	}
}

void UMASkillBehavior::HandleVFXSpawnEvent(FGameplayEventData EventData)
{
	FGameplayAbilityActivationInfo ActivationInfo = OwningAbility->GetCurrentActivationInfo();
	if (!OwningAbility || !OwningAbility->HasAuthority(&ActivationInfo))
		return;

	if (!VFXDataSet)
		return;

	//Event.VFX.~ 태그와 동일한 Value 구조체를 가져와
	const F_SkillVFX_Info* VFXInfo = VFXDataSet->VFXDataMap.Find(EventData.EventTag);
	if (!VFXInfo || !VFXInfo->DefaultVFX)
		return;

	TObjectPtr<UNiagaraSystem> FinalVFXToSpawn = VFXInfo->DefaultVFX;
	FLinearColor SpawnColor = FLinearColor::White;
	bool bApplyColor = false;
	//속성 태그 - Linear Color 로 구성된 데이터 테이블
	const UDataTable* ElementDT = OwningAbility->GetElementDataTable();
	//현재 스킬의 속성 태그 가져와
	FGameplayTag ElementTag = OwningAbility->GetSkillElementTag();
	
	if (ElementTag.IsValid())
	{	//속성 태그의 마지막 단어 획득
		
		TArray<FName> TagNames;
		UGameplayTagsManager::Get().SplitGameplayTagFName(ElementTag, TagNames);
		FName LastName = TagNames.Last();

		const TObjectPtr<UNiagaraSystem>* OverrideVFX = VFXInfo->ElementVFXOverride.Find(LastName);
		//오버라이드할 VFX 설정했다면
		if (OverrideVFX && *OverrideVFX)
		{
			FinalVFXToSpawn = *OverrideVFX;
			bApplyColor = false;
		}
		//오버라이드할 VFX 없다면
		else if (VFXInfo->bUseElementColor && ElementDT)
		{
			//가져온 속성 태그에 해당하는 행
			F_ElementInfoRow* ElementInfo = ElementDT->FindRow<F_ElementInfoRow>(LastName, "");
			if (ElementInfo)
			{
				//가져온 행에서 색상정보 가져와
				SpawnColor = ElementInfo->ElementColor;
				bApplyColor = true;
			}
		}
	}
	USkeletalMeshComponent* MeshComp = Character->GetMesh();
	if (!MeshComp)
		return;
		
	UNiagaraComponent* SpawnedVFX = nullptr;
	if (VFXInfo->bSpawnInWorld)
	{
		FTransform SocketTransform = (VFXInfo->SocketName != NAME_None) ?
		MeshComp->GetSocketTransform(VFXInfo->SocketName) : MeshComp->GetComponentTransform();
			
		FTransform OffsetTransform(VFXInfo->RotationOffset,VFXInfo->LocationOffset, VFXInfo->Scale);
		FTransform WorldSpawnTransform = OffsetTransform * SocketTransform;
		
		Character->Multicast_PlayNiagara(FinalVFXToSpawn, WorldSpawnTransform, bApplyColor, SpawnColor);
	}
	else
	{
		Character->Multicast_PlayNiagaraAttached(
			FinalVFXToSpawn,
			VFXInfo->SocketName,
			VFXInfo->LocationOffset,
			VFXInfo->RotationOffset,
			VFXInfo->Scale,
			VFXInfo->bAutoDestroy,
			bApplyColor, SpawnColor);
	}
	if (SpawnedVFX && bApplyColor)
	{
		SpawnedVFX->SetVariableLinearColor(FName("EffectColor"),SpawnColor);
	}
}

void UMASkillBehavior::SafeEndAbility()
{
	if (OwningAbility)
		OwningAbility->EndAbility(OwningAbility->GetCurrentAbilitySpecHandle(),OwningAbility->GetCurrentActorInfo(),
			OwningAbility->GetCurrentActivationInfo(),true,false);
}

void UMASkillBehavior::SetMontagePlayRate(float NewPlayRate)
{
	if (Character)
	{
		if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
		{
			UAnimMontage* ActiveMontage  = AnimInstance->GetCurrentActiveMontage();
			AnimInstance->Montage_SetPlayRate(ActiveMontage ,NewPlayRate);
		}
	}
}

void UMASkillBehavior::MontageToOtherSection(FName SectionName)
{
	if (Character)
	{
		if (UAnimInstance* AnimInst = Character->GetMesh()->GetAnimInstance())
		{
			AnimInst->Montage_JumpToSection(SectionName,MontageToPlay);
		}
	}
}

class AMACharacter* UMASkillBehavior::GetCharacter() const
{
	if (OwningAbility)
	{
		return Cast<AMACharacter>(OwningAbility->GetAvatarActorFromActorInfo());
	}
	return nullptr;
}
