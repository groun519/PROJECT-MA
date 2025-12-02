// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_SpawnActorFwd.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"
#include "SkillBehaviorConfig.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "GameFramework/PlayerController.h"
#include "Character/MACharacter.h"
#include "GAS/Projectile/MAProjectile_OverlapAOE.h"
#include "GAS/UtilityModule/UtilityModule.h"


void USkillBehavior_SpawnActorFwd::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();
	if (!OwningAbility || !Character || !DefaultProjectile)
		return;

	FGameplayAbilityActivationInfo ActivationInfo = OwningAbility->GetCurrentActivationInfo();
	if (OwningAbility->HasAuthorityOrPredictionKey(OwningAbility->GetCurrentActorInfo(), &ActivationInfo))
	{
		ProjectileEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, ProjectileTag);
		ProjectileEventTask->EventReceived.AddDynamic(this, &USkillBehavior_SpawnActorFwd::OnProjectileEventReceived);
		ProjectileEventTask->ReadyForActivation();
	}
}

void USkillBehavior_SpawnActorFwd::OnEndAbility_Implementation()
{
	if (ProjectileEventTask.IsValid())
		ProjectileEventTask->EndTask();
	
	Super::OnEndAbility_Implementation();
}

void USkillBehavior_SpawnActorFwd::InitFromConfig(const FInstancedStruct& ConfigPayload)
{
	Super::InitFromConfig(ConfigPayload);
	const FConfig_SpawnActorAtFwd* ConfigSpawn = ConfigPayload.GetPtr<FConfig_SpawnActorAtFwd>();
	if (ConfigSpawn)
	{
		DefaultProjectile = ConfigSpawn->DefaultProjectile;
		ElementalProjectiles = ConfigSpawn->ElementalProjectiles;
		ProjectileMaxDist=ConfigSpawn->MaxDistance;
		ProjectileSpeed=ConfigSpawn->ProjectileSpeed;
		ExplodeRadius=ConfigSpawn->ExplodeRadius;
		SpawnDelay=ConfigSpawn->SpawnDelay;
		ProjectileCount=ConfigSpawn->ProjectileCount;
		MuzzleSocketName=ConfigSpawn->MuzzleSocketName;
	}
}

void USkillBehavior_SpawnActorFwd::OnProjectileEventReceived(FGameplayEventData EventData)
{
	if (OwningAbility->K2_HasAuthority())
	{
		//속성별 투사체 설정
		ProjectileToSpawn = DefaultProjectile;
		FGameplayTag ElementTag = OwningAbility->GetSkillElementTag();
		if (ElementTag.IsValid())
		{
			TArray<FName> TagNames;
			UGameplayTagsManager::Get().SplitGameplayTagFName(ElementTag, TagNames);
			FName AttributeName = TagNames.Last();

			if (ElementalProjectiles.FindRef(AttributeName))
			{
				ProjectileToSpawn = ElementalProjectiles.FindRef(AttributeName);
			}
		}
		
		AActor* OwnerAvatarActor = OwningAbility->GetAvatarActorFromActorInfo();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerAvatarActor;
		SpawnParams.Instigator = Cast<APawn>(OwnerAvatarActor);
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		FGameplayEffectSpecHandle SpecHandle = OwningAbility->MakeOutgoingGameplayEffectSpec(OwningAbility->GetBaseDamageEffect());

		//유틸리티 데미지 배율
		if (OwningAbility->GetActiveUtilityModule())
		{
			OwningAbility->GetActiveUtilityModule()->ModifyDamageEffectSpec(SpecHandle);
		}
		//속성 데미지 배율
		const F_ElementInfoRow* ElementInfoRow = OwningAbility->GetActiveElementInfoRow();
		if (ElementInfoRow && ElementInfoRow->ElementalDamageMultiplier != 1.f)
		{
			SpecHandle.Data->SetSetByCallerMagnitude(
				UMAAbilitySystemStatics::GetElementalMultiplierTag(),
				ElementInfoRow->ElementalDamageMultiplier);
		}
		//행동 데미지 배율
		SpecHandle.Data->SetSetByCallerMagnitude(UMAAbilitySystemStatics::GetBehaviorMultiplierTag(),BehaviorDamageMultiplier);

		USkeletalMeshComponent* Mesh = Character->GetMesh();
		if (!Mesh)
			return;
		
		FVector MuzzleLocation;
		if (!MuzzleSocketName.IsValid() || !Mesh->DoesSocketExist(MuzzleSocketName))
		{
			MuzzleLocation = Character->GetActorLocation();
		}else
		{
			MuzzleLocation = Mesh->GetSocketTransform(MuzzleSocketName).GetLocation();
		}

		AMAProjectile_OverlapAOE* OverlapProjectile = GetWorld()->SpawnActor<AMAProjectile_OverlapAOE>(ProjectileToSpawn,MuzzleLocation, OwnerAvatarActor->GetActorRotation(), SpawnParams);
		if (OverlapProjectile)
		{
			if (ElementInfoRow->ElementEffect)
			{	//속성 추가 효과 적용
				OverlapProjectile->AdditionalEffect = ElementInfoRow->ElementEffect;
			}
			OverlapProjectile->ShootProjectile(ProjectileSpeed, ProjectileMaxDist, ExplodeRadius,OwningAbility->GetOwnerTeamId(),SpecHandle);
		}
	}
}



