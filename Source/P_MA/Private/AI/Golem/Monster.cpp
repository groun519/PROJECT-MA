// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Golem/Monster.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAGameplayEffect_MonsterWaveStatScale.h"

void AMonster::BeginPlay()
{
	Super::BeginPlay();
	ApplyEnvMaterials();
}

void AMonster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMonster, EnvGameplayTag);
}

void AMonster::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	ApplyStatCoefficientEffect();
}

void AMonster::SetGenericTeamId(const FGenericTeamId& NewTeamId)
{
	Super::SetGenericTeamId(NewTeamId);
}

void AMonster::SetEnvTag(const FGameplayTag& InEnvTag)
{
	if (EnvGameplayTag == InEnvTag) return;

	EnvGameplayTag = InEnvTag;
	ApplyEnvMaterials();

	if (HasAuthority())
	{
		ForceNetUpdate();
	}
}

bool AMonster::IsActive() const
{
	return bActiveInPool && !IsDead();
}

void AMonster::Activate()
{
	GetWorldTimerManager().ClearTimer(DisappearTimerHandle);

	bActiveInPool = true;

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}

	RespawnImmediately();

	if (AController* BaseCon = GetController())
	{
		if (AAIController* AICon = Cast<AAIController>(BaseCon))
		{
			if (UBrainComponent* Brain = AICon->GetBrainComponent())
			{
				Brain->StartLogic();
			}
		}
	}
}

void AMonster::Deactivate()
{
	if (HasAuthority())
	{
		if (UMAAbilitySystemComponent* ASC = Cast<UMAAbilitySystemComponent>(GetAbilitySystemComponent()))
		{
			ASC->RemoveLooseGameplayTags(ASC->AppliedBaseTags);
			ASC->AppliedBaseTags.Reset();
		}
	}
	bActiveInPool = false;

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}

	if (AController* BaseCon = GetController())
	{
		if (AAIController* AICon = Cast<AAIController>(BaseCon))
		{
			if (UBrainComponent* Brain = AICon->GetBrainComponent())
			{
				Brain->StopLogic(TEXT("Monster Deactivated"));
			}
		}
	}
}

void AMonster::ApplyStatCoefficientEffect()
{
	if (!HasAuthority()) return;
	if (FMath::IsNearlyEqual(StatCoefficient, 1.f)) return;

	UMAAbilitySystemComponent* ASC = Cast<UMAAbilitySystemComponent>(GetAbilitySystemComponent());
	if (!ASC) return;

	UMAGameplayEffect_MonsterWaveStatScale::ApplyTo(*ASC, StatCoefficient);
	ASC->ApplyFullStatEffect();
}

void AMonster::ApplyEnvMaterials()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return;
	}

	const FMonsterEnvData* Found = nullptr;
	for (const FMonsterEnvData& Data : EnvTagToMaterial)
	{
		if (Data.EnvTag == EnvGameplayTag)
		{
			Found = &Data;
			break;
		}
	}
	if (!Found) return;

	const TArray<UMaterialInterface*>& MIList = Found->MIList;
	for (int32 Index = 0; Index < MIList.Num(); ++Index)
	{
		if (MIList[Index])
		{
			MeshComp->SetMaterial(Index, MIList[Index]);
		}
	}
}

void AMonster::SetGoal(AActor* Goal)
{
	if (AAIController* AIController = GetController<AAIController>())
	{
		if (UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent())
		{
			BlackboardComponent->SetValueAsObject(GoalBlackboardKeyName, Goal);
		}
	}
}

void AMonster::OnRep_TeamID()
{
	
}

void AMonster::OnRep_EnvGameplayTag()
{
	ApplyEnvMaterials();
}

void AMonster::OnDead()
{
	Super::OnDead();
	OnMonsterDead.Broadcast();
	
	if (HasAuthority())
	{
		GetWorldTimerManager().ClearTimer(DisappearTimerHandle);
		GetWorldTimerManager().SetTimer(
			DisappearTimerHandle,
			this, &AMonster::Deactivate,DisappearDelay,
			false
		);
	}
}
