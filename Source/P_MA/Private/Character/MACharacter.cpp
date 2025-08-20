// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/MACharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "Kismet/GameplayStatics.h"

#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"


AMACharacter::AMACharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MAAbilitySystemComponent = CreateDefaultSubobject<UMAAbilitySystemComponent>("MAAbility System Component");
	MAAttributeSet = CreateDefaultSubobject<UMAAttributeSet>("MAAttribute Set");

	BindGASChangeDelegates();
	
	PerceptionStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("Perception Stimuli Source Component");
}

void AMACharacter::ServerSideInit()
{
	MAAbilitySystemComponent->InitAbilityActorInfo(this, this);
	MAAbilitySystemComponent->ApplyInitialEffects();
	MAAbilitySystemComponent->GiveInitialAbilities();
}

void AMACharacter::ClientSideInit()
{
	MAAbilitySystemComponent->InitAbilityActorInfo(this, this);
}

bool AMACharacter::IsLocallyControlledByPlayer() const
{
	return GetController() && GetController()->IsLocalPlayerController();
}

void AMACharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AMACharacter::BeginPlay()
{
	Super::BeginPlay();

	MeshRelativeTransform = GetMesh()->GetRelativeTransform();
	PerceptionStimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
}

void AMACharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
}

UAbilitySystemComponent* AMACharacter::GetAbilitySystemComponent() const
{
	return MAAbilitySystemComponent;
}

void AMACharacter::BindGASChangeDelegates()
{
	if (MAAbilitySystemComponent)
	{
		MAAbilitySystemComponent->RegisterGameplayTagEvent(UMAAbilitySystemStatics::GetDeadStatTag()).AddUObject(this, &ACCharacter::DeathTagUpdated);
	}
}

void AMACharacter::DeathTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount != 0)
	{
		StartDeathSequence();
	}
	else
	{
		Respawn();
	}
}

void AMACharacter::SetStatusGaugeEnabled(bool bIsEnabled)
{
	GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityUpdateTimerHandle);
	if (bIsEnabled)
	{
		// TODO:
		//ConfigureOverHeadStatusWidget();
	}
	else
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);
	}
}

void AMACharacter::DeathMontageFinished()
{
	SetRagdollEnabled(true);
}

void AMACharacter::SetRagdollEnabled(bool bIsEnabled)
{
	if (bIsEnabled)
	{
		GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}
	else
	{
		GetMesh()->SetSimulatePhysics(false);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		GetMesh()->SetRelativeTransform(MeshRelativeTransform);
	}
}

void AMACharacter::PlayDeathAnimation()
{
	if (DeathMontage)
	{
		float MontageDuration = PlayAnimMontage(DeathMontage);
		GetWorldTimerManager().SetTimer(DeathMontageTimerHandle, this, &AMACharacter::DeathMontageFinished, MontageDuration + DeathMontageFinishTimeShift);
	}
}

void AMACharacter::StartDeathSequence()
{
	OnDead();
	PlayDeathAnimation();
	SetStatusGaugeEnabled(false);

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMACharacter::Respawn()
{
	OnRespawn();
	//SetRagdollEnabled(false); <- 래그돌이 안 되는 쪽이 어울릴 것 같음.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetMesh()->GetAnimInstance()->StopAllMontages(0.f);
	SetStatusGaugeEnabled(true);

	if (MAAbilitySystemComponent)
	{
		MAAbilitySystemComponent->ApplyFullStatEffect();
	}
}

void AMACharacter::OnDead()
{
}

void AMACharacter::OnRespawn()
{
}

void AMACharacter::SetAIPerceptionStimuliSourceEnabled(bool bIsEnabled)
{
	if (!PerceptionStimuliSourceComponent)
	{
		return;
	}

	if (bIsEnabled)
	{
		PerceptionStimuliSourceComponent->RegisterWithPerceptionSystem();
	}
	else
	{
		PerceptionStimuliSourceComponent->UnregisterFromPerceptionSystem();
	}
}

void AMACharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

