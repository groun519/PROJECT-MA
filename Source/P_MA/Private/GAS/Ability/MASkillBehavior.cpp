// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/MASkillBehavior.h"

#include "GameplayTagsManager.h"
#include "GAS/MASkillVFXSet.h"
#include "MAGameplayAbility_SkillBase.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
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
}

void UMASkillBehavior::OnEndAbility_Implementation()
{
	if (WaitVFXEventTask.IsValid())
		WaitVFXEventTask->EndTask();
	
	this->Character = nullptr;
	this->PlayerCharacter = nullptr;
}

void UMASkillBehavior::ApplyCooldownAndEndAbility(TSubclassOf<UGameplayEffect> CooldownEffect)
{
	if (!OwningAbility)
		return;
	if (CooldownEffect)
	{
		OwningAbility->ApplyEffectToOwner(CooldownEffect);
	}
	UAbilityTask_WaitDelay* EndDelayTask = UAbilityTask_WaitDelay::WaitDelay(OwningAbility, 0.05f);
	EndDelayTask->OnFinish.AddDynamic(this, &UMASkillBehavior::SafeEndAbility);
	EndDelayTask->ReadyForActivation();
}

void UMASkillBehavior::HandleVFXSpawnEvent(FGameplayEventData EventData)
{
	FGameplayAbilityActivationInfo ActivationInfo = OwningAbility->GetCurrentActivationInfo();
	if (!OwningAbility || !OwningAbility->HasAuthority(&ActivationInfo))
		return;

	if (!VFXDataSet)
		return;

	const F_SkillVFX_Info* VFXInfo = VFXDataSet->VFXDataMap.Find(EventData.EventTag);
	if (!VFXInfo || !VFXInfo->VFXToSpawn)
		return;

	FLinearColor SpawnColor = FLinearColor::White;
	bool bApplyColor = false;
	const UDataTable* ElementDT = OwningAbility->GetElementDataTable();
	if (VFXInfo->bUseElementColor && ElementDT)
	{
		FGameplayTag ElementTag = OwningAbility->GetSkillElementTag();
		if (ElementTag.IsValid())
		{
			TArray<FName> TagNames;
			UGameplayTagsManager::Get().SplitGameplayTagFName(ElementTag, TagNames);
			FName LastName = TagNames.Last();

			F_ElementInfoRow* ElementInfo = ElementDT->FindRow<F_ElementInfoRow>(LastName, TEXT("ElementDataLookup"));
			if (ElementInfo)
			{
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
		
		Character->Multicast_PlayNiagara(VFXInfo->VFXToSpawn, WorldSpawnTransform, bApplyColor, SpawnColor);
	}
	else
	{
		Character->Multicast_PlayNiagaraAttached(
			VFXInfo->VFXToSpawn,
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
		OwningAbility->RequestEndAbility();
}

class AMACharacter* UMASkillBehavior::GetCharacter() const
{
	if (OwningAbility)
	{
		return Cast<AMACharacter>(OwningAbility->GetAvatarActorFromActorInfo());
	}
	return nullptr;
}
