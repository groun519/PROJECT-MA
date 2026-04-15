#include "GAS/Ability/GA_BasicAttack_Melee.h"

#include "GameplayTagsManager.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Character/MACharacter.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MASkillVFXSet.h"

UGA_BasicAttack_Melee::UGA_BasicAttack_Melee()
{
	AbilityTags.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
	BlockAbilitiesWithTag.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGA_BasicAttack_Melee::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                            const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage);
		PlayMontageTask->OnBlendOut.AddDynamic(this, &UGA_BasicAttack_Melee::K2_EndAbility);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_BasicAttack_Melee::K2_EndAbility);
		PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_BasicAttack_Melee::K2_EndAbility);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_BasicAttack_Melee::K2_EndAbility);
		PlayMontageTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitComboChangeTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Ability.Combo.Change"),nullptr, false,false);
		WaitComboChangeTask->EventReceived.AddDynamic(this, &UGA_BasicAttack_Melee::ComboChangeEventReceived);
		WaitComboChangeTask->ReadyForActivation();
	}

	if (K2_HasAuthority())
	{
		UAbilityTask_WaitGameplayEvent* WaitDamageTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Event.Montage.Damage"));
		WaitDamageTask->EventReceived.AddDynamic(this, &UGA_BasicAttack_Melee::DoDamage);
		WaitDamageTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitClearTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Ability.Combo.Clear"));
		WaitClearTask->EventReceived.AddDynamic(this, &UGA_BasicAttack_Melee::ClearIgnore);
		WaitClearTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitVFXTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Event.VFX"),nullptr, false,false);
		WaitVFXTask->EventReceived.AddDynamic(this, &UGA_BasicAttack_Melee::HandleVFXEvent);
		WaitVFXTask->ReadyForActivation();
	}
	SetupWaitInputPress();
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_BasicAttack_Melee::ComboChangeEventReceived(FGameplayEventData Payload)
{
	FGameplayTag EventTag= Payload.EventTag;
	if (EventTag == FGameplayTag::RequestGameplayTag("Ability.Combo.Change.End"))
	{
		NextComboName = NAME_None;
		return;
	}
	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);
	NextComboName = TagNames.Last();
}

void UGA_BasicAttack_Melee::DoDamage(FGameplayEventData Payload)
{
	TArray<FHitResult> HitResults = GetHitResultFromVirtualSocketTargetData(Payload.TargetData);

	for (const FHitResult& HitResult : HitResults)
	{
		if (IgnoreActors.Contains(HitResult.GetActor()))
			continue;

		TSubclassOf<UGameplayEffect> DamageGE = nullptr;
		UMAAbilitySystemComponent* ASC = Cast<UMAAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
		if (ASC && ASC->GetSystemGenerics())
		{
			DamageGE = ASC->GetSystemGenerics()->GetDamageEffect();
		}
		ApplyGameplayEffectToHitResultActor(HitResult, DamageGE, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		IgnoreActors.Add(HitResult.GetActor());
	}
}

void UGA_BasicAttack_Melee::ClearIgnore(FGameplayEventData Payload)
{
	IgnoreActors.Empty();
}

void UGA_BasicAttack_Melee::HandleInputPress(float TimeWaited)
{
	SetupWaitInputPress();
	TryCommitCombo();
}

void UGA_BasicAttack_Melee::HandleVFXEvent(FGameplayEventData Payload)
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

void UGA_BasicAttack_Melee::SetupWaitInputPress()
{
	UAbilityTask_WaitInputPress* WaitInputPress = UAbilityTask_WaitInputPress::WaitInputPress(this);
	WaitInputPress->OnPress.AddDynamic(this, &UGA_BasicAttack_Melee::HandleInputPress);
	WaitInputPress->ReadyForActivation();
}

void UGA_BasicAttack_Melee::TryCommitCombo()
{
	if (NextComboName == NAME_None)
		return;

	UAnimInstance* OwnerAnimInst = GetOwnerAnimInstance();
	if (!OwnerAnimInst)
		return;

	OwnerAnimInst->Montage_SetNextSection(OwnerAnimInst->Montage_GetCurrentSection(AttackMontage), NextComboName, AttackMontage);
}
