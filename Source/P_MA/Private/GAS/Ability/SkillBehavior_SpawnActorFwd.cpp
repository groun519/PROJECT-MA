// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_SpawnActorFwd.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "GAS/Projectile/MAProjectile_OverlapAOE.h"
#include "GameFramework/PlayerController.h"
#include "Character/MACharacter.h"


void USkillBehavior_SpawnActorFwd::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();
	if (!OwningAbility || !Character || !ProjectileClass)
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


void USkillBehavior_SpawnActorFwd::OnProjectileEventReceived(FGameplayEventData EventData)
{
	if (OwningAbility->K2_HasAuthority())
	{
		AActor* OwnerAvatarActor = OwningAbility->GetAvatarActorFromActorInfo();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerAvatarActor;
		SpawnParams.Instigator = Cast<APawn>(OwnerAvatarActor);
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

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

		AMAProjectile_OverlapAOE* OverlapProjectile = GetWorld()->SpawnActor<AMAProjectile_OverlapAOE>(
			ProjectileClass,MuzzleLocation, OwnerAvatarActor->GetActorRotation(), SpawnParams);
		if (OverlapProjectile)
		{
			OverlapProjectile->ShootProjectile(ProjectileSpeed, ProjectileMaxDist, ExplodeRadius,
				OwningAbility->GetOwnerTeamId(),OwningAbility->MakeOutgoingGameplayEffectSpec(DamageEffect));
		}
	}

	if (CooldownGE)
		OwningAbility->ApplyEffectToOwner(CooldownGE);
}



