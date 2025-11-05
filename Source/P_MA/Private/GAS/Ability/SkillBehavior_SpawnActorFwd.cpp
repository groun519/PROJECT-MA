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
	
	//애니메이션에서 발사 노티파이 대기
	ProjectileEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, ProjectileTag);
	ProjectileEventTask->EventReceived.AddDynamic(this, &USkillBehavior_SpawnActorFwd::OnProjectileEventReceived);
	ProjectileEventTask->ReadyForActivation();
}

void USkillBehavior_SpawnActorFwd::OnEndAbility_Implementation()
{
	if (CooldownGE)
		OwningAbility->ApplyEffectToOwner(CooldownGE);
	if (ProjectileEventTask.IsValid())
		ProjectileEventTask->EndTask();
	
	Super::OnEndAbility_Implementation();
}


void USkillBehavior_SpawnActorFwd::OnProjectileEventReceived(FGameplayEventData EventData)
{
	if (!Character || !ProjectileClass)
		return;
	if (Character->IsLocallyControlled())
	{
		USkeletalMeshComponent* Mesh = Character->GetMesh();
		if (!Mesh || !Mesh->DoesSocketExist(MuzzleSocketName))
			return;
		
		const FVector MuzzleLocation = Mesh->GetSocketTransform(MuzzleSocketName).GetLocation();
		const FVector TargetDirection = Character->GetActorForwardVector();
		const FRotator FinalSpawnRotation = TargetDirection.Rotation();

		Character->Server_SpawnOverlapAoEProjectile(ProjectileClass, MuzzleLocation, FinalSpawnRotation,AbilitySize);
	}
}



