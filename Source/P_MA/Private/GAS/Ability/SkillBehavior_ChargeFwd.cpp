// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_ChargeFwd.h"

#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"
#include "MAGameplayAbility_SkillBase.h"
#include "SkillBehaviorConfig.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Character/MACharacter.h"
#include "GAS/MASkillVFXSet.h"


void USkillBehavior_ChargeFwd::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();
	if (!Character || !TargetActorClass)
		return;
	OwningAbility->GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(UMAAbilitySystemStatics::GetMoveBlockTag());
	CachedChargeDuration=0.f;
	TargetActor = GetWorld()->SpawnActor<AMATargetActor_ChargeAtFwd>(TargetActorClass);
	if (TargetActor)
	{
		TargetActor->AttachToActor(Character, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		TargetActor->StartTargeting(OwningAbility);
		TargetActor->Initialize(MaxTraceDistance, MinTraceDistance, SkillWidth, DecalDepth, MaxChargeDuration);
	}
	
	InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(OwningAbility);
	InputReleaseTask->OnRelease.AddDynamic(this, &USkillBehavior_ChargeFwd::OnKeyReleased);
	InputReleaseTask->ReadyForActivation();

	SkillTimeoutTask= UAbilityTask_WaitDelay::WaitDelay(OwningAbility, MaxChargeDuration+0.5f);
	SkillTimeoutTask->OnFinish.AddDynamic(this, &USkillBehavior_ChargeFwd::OnSkillTimeout);
	SkillTimeoutTask->ReadyForActivation();
}

void USkillBehavior_ChargeFwd::OnEndAbility_Implementation()
{
	CleanUp();
	CachedChargeDuration=0.f;
	if (InputReleaseTask.IsValid())
		InputReleaseTask->EndTask();
	if (SkillTimeoutTask.IsValid())
		SkillTimeoutTask->EndTask();

	Super::OnEndAbility_Implementation();
}

float USkillBehavior_ChargeFwd::GetCurrentDamageMultiplier() const
{
	return CachedChargeDuration;
}

void USkillBehavior_ChargeFwd::InitFromConfig(const FInstancedStruct& ConfigPayload)
{
	Super::InitFromConfig(ConfigPayload);
	const FConfig_ChargeFwd* ChargeFwdConfig = ConfigPayload.GetPtr<FConfig_ChargeFwd>();
	if (ChargeFwdConfig)
	{
		TargetActorClass = ChargeFwdConfig->TargetActorClass;
		MaxChargeDuration = ChargeFwdConfig->MaxChargeDuration;
		MaxTraceDistance = ChargeFwdConfig->MaxTraceDistance;
		MinTraceDistance = ChargeFwdConfig->MinTraceDistance;
	}
}

void USkillBehavior_ChargeFwd::OnKeyReleased(float TimeHeld)
{
	if (!TargetActor || !OwningAbility)
		return;
	if (TimeHeld <= 0.2f)
	{
		CleanUp();
		OwningAbility->ApplyShortCooldownAndRequestEndAbility();
		return;
	}
	CachedChargeDuration=TimeHeld;
	float ChargeRatio = FMath::Clamp(TimeHeld / MaxChargeDuration, 0.f, 1.f);
	float FinalLength = FMath::Lerp(MinTraceDistance, MaxTraceDistance, ChargeRatio);
	SpawnVFX(FinalLength);
	
	FGameplayAbilityTargetDataHandle TargetDataHandle = TargetActor->GetTargetData();
	if (OwningAbility->K2_HasAuthority())
		OwningAbility->ApplyDamageToTargetData(TargetDataHandle);
	
	CleanUp();
	SafeEndAbility();
	OwningAbility->ApplyDefaultCooldownOnce();
}

void USkillBehavior_ChargeFwd::OnSkillTimeout()
{
	CleanUp();
	OwningAbility->ApplyShortCooldownAndRequestEndAbility();
}

void USkillBehavior_ChargeFwd::SpawnVFX(float FinalLength)
{
	if (!OwningAbility || !VFXDataSet ||!Character)
		return;

	const F_SkillVFX_Info* VFXInfo = VFXDataSet->VFXDataMap.Find(FGameplayTag::RequestGameplayTag("Event.VFX.Attack1"));
	if (!VFXInfo || !VFXInfo->DefaultVFX)
		return;

	TObjectPtr<UNiagaraSystem> FinalVFXToSpawn = VFXInfo->DefaultVFX;

	FGameplayTag ElementTag = OwningAbility->GetSkillElementTag();
	if (ElementTag.IsValid())
	{
		TArray<FName> TagNames;
		UGameplayTagsManager::Get().SplitGameplayTagFName(ElementTag, TagNames);
		FName LastName = TagNames.Last();

		const TObjectPtr<UNiagaraSystem>* OverrideVFX = VFXInfo->ElementVFXOverride.Find(LastName);
		//오버라이드할 VFX 설정했다면
		if (OverrideVFX && *OverrideVFX)
		{
			FinalVFXToSpawn = *OverrideVFX;
		}
	}
	FVector Location = Character->GetActorLocation();
	FRotator Rotation = Character->GetActorRotation();

	float SafeLength = (VFXLength == 0.f) ? 1.f : VFXLength;
	float SafeWidth = (VFXWidth == 0.f) ? 1.f : VFXWidth;

	FVector Scale = FVector (FinalLength / SafeLength, SkillWidth/SafeWidth, 1.f);
	FTransform SpawnTransform(Rotation,Location,Scale);

	Character->Multicast_PlayNiagara(FinalVFXToSpawn,SpawnTransform);
}

void USkillBehavior_ChargeFwd::CleanUp()
{
	OwningAbility->GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(UMAAbilitySystemStatics::GetMoveBlockTag());
	if (TargetActor)
	{
		TargetActor->Destroy();
		TargetActor=nullptr;
	}
}
