// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/MACharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Character/MAReactionComponent.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Widget/MAOverHeadStatsGauge.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "P_MA/P_MA.h"
#include "Player/Loadout/LoadoutComponent.h"

AMACharacter::AMACharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	/** Mesh **/
	GetMesh()->SetupAttachment(GetRootComponent());
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	/****/

	/** CapsuleComp **/
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Target, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionObjectType(ECC_Hitbox);
	/****/
	
	MAAbilitySystemComponent = CreateDefaultSubobject<UMAAbilitySystemComponent>("MAAbility System Component");
	MAAttributeSet = CreateDefaultSubobject<UMAAttributeSet>("MAAttribute Set");
	ReactionComponent = CreateDefaultSubobject<UMAReactionComponent>("Reaction Component");
	OverHeadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("Over Head Widget Component");
	OverHeadWidgetComponent->SetupAttachment(GetRootComponent());
	LoadoutComponent = CreateDefaultSubobject<ULoadoutComponent>("LoadoutComponent");

	BindGASChangeDelegates();

	PerceptionStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("Perception Stimuli Source Component");
}

void AMACharacter::ServerSideInit()
{
	MAAbilitySystemComponent->InitAbilityActorInfo(this, this);
	MAAbilitySystemComponent->ServerSideInit();
}

void AMACharacter::ClientSideInit()
{
	MAAbilitySystemComponent->InitAbilityActorInfo(this, this);
}

bool AMACharacter::IsLocallyControlledByPlayer() const
{
	return GetController() && GetController()->IsLocalPlayerController();
}

void AMACharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMACharacter, TeamID);
}

const TMap<EMAAbilityInputID, TSubclassOf<UGameplayAbility>>& AMACharacter::GetAbilities() const
{
	return MAAbilitySystemComponent->GetAbilities();
}

void AMACharacter::BeginPlay()
{
	Super::BeginPlay();
	ConfigureOverHeadStatusWidget();

	MeshRelativeTransform = GetMesh()->GetRelativeTransform();

	PerceptionStimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());

	if (LoadoutComponent)
	{
		LoadoutComponent->InitializeMaterial(GetMesh());
	}
}

void AMACharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (NewController && !NewController->IsPlayerController())
	{
		ServerSideInit();
	}
}

void AMACharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID;
}

FGenericTeamId AMACharacter::GetGenericTeamId() const
{
	return TeamID;
}

UAbilitySystemComponent* AMACharacter::GetAbilitySystemComponent() const
{
	return MAAbilitySystemComponent;
}

void AMACharacter::Server_SendGameplayEventToSelf_Implementation(const FGameplayTag& EventTag,
                                                                 const FGameplayEventData& EventData)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventTag, EventData);
}

bool AMACharacter::Server_SendGameplayEventToSelf_Validate(const FGameplayTag& EventTag,
	const FGameplayEventData& EventData)
{
	return true;
}

void AMACharacter::BindGASChangeDelegates()
{
	if (MAAbilitySystemComponent)
	{
		MAAbilitySystemComponent->RegisterGameplayTagEvent(UMAAbilitySystemStatics::GetDeadStatTag()).AddUObject(this, &AMACharacter::DeathTagUpdated);
		MAAbilitySystemComponent->RegisterGameplayTagEvent(UMAAbilitySystemStatics::GetAimingTag()).AddUObject(this, &AMACharacter::AimTagUpdated);
		MAAbilitySystemComponent->RegisterGameplayTagEvent(UMAAbilitySystemStatics::GetMoveBlockTag()).AddUObject(this, &AMACharacter::MoveBlockTagUpdated);
		MAAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetMoveSpeedAttribute()).AddUObject(this, &AMACharacter::MoveSpeedUpdated);
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

void AMACharacter::AimTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (IsDead()) return;
	RefreshMaxWalkSpeed();
}

void AMACharacter::MoveBlockTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (IsDead()) return;
	if (NewCount != 0) StopMovementForBlock();
	RefreshMaxWalkSpeed();
}

void AMACharacter::MoveSpeedUpdated(const FOnAttributeChangeData& Data)
{
	RefreshMaxWalkSpeed();
}

void AMACharacter::SetStatusGaugeEnabled(bool bIsEnabled)
{
	GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityUpdateTimerHandle);
	if (!bIsEnabled)
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);
	}
}

bool AMACharacter::IsDead() const
{
	return GetAbilitySystemComponent() -> HasMatchingGameplayTag(UMAAbilitySystemStatics::GetDeadStatTag());
}

bool AMACharacter::IsMovementBlocked() const
{
	return MAAbilitySystemComponent && MAAbilitySystemComponent->HasMatchingGameplayTag(UMAAbilitySystemStatics::GetMoveBlockTag());
}

void AMACharacter::RefreshMaxWalkSpeed()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp || !MAAttributeSet || !MAAbilitySystemComponent) return;

	const float MoveSpeed = MAAttributeSet->GetMoveSpeed();
	if (MAAbilitySystemComponent->HasMatchingGameplayTag(UMAAbilitySystemStatics::GetMoveBlockTag()))
	{
		MoveComp->MaxWalkSpeed = 0.f;
		return;
	}

	if (MAAbilitySystemComponent->HasMatchingGameplayTag(UMAAbilitySystemStatics::GetAimingTag()))
	{
		MoveComp->MaxWalkSpeed = MoveSpeed * 0.2f;
		return;
	}

	MoveComp->MaxWalkSpeed = MoveSpeed;
}

void AMACharacter::StopMovementForBlock()
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
	}
}

void AMACharacter::RespawnImmediately()
{
	if (HasAuthority())
	{
		GetAbilitySystemComponent()->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(UMAAbilitySystemStatics::GetDeadStatTag()));
	}
}

void AMACharacter::PlayDeathAnimation()
{
	if (DeathMontage)
	{
		float MontageDuration = PlayAnimMontage(DeathMontage);
	}
}

void AMACharacter::StartDeathSequence()
{
	OnDead();

	if (ReactionComponent)
	{
		ReactionComponent->ResetTransientReactionState();
	}

	if (MAAbilitySystemComponent)
	{
		MAAbilitySystemComponent->CancelAllAbilities();
	}
	
	PlayDeathAnimation();
	SetStatusGaugeEnabled(false);

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetAIPerceptionStimuliSourceEnabled(false);
}

void AMACharacter::Respawn()
{
	if (ReactionComponent)
	{
		ReactionComponent->ResetTransientReactionState();
	}

	SetAIPerceptionStimuliSourceEnabled(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
		{
			AnimInst->StopAllMontages(0.f);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Respawn: AnimInstance is null. Char=%s"), *GetName());
		}
	}
	SetStatusGaugeEnabled(true);

	if (MAAbilitySystemComponent)
	{
		MAAbilitySystemComponent->ApplyFullStatEffect();
	}

	OnRespawn();
}

void AMACharacter::OnDead()
{
}

void AMACharacter::OnRespawn()
{
}

void AMACharacter::OnRep_TeamID()
{
	// override only
}

void AMACharacter::SetAIPerceptionStimuliSourceEnabled(bool bIsEnabled)
{
	if (!PerceptionStimuliSourceComponent) return;

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

void AMACharacter::ConfigureOverHeadStatusWidget()
{
	if (!OverHeadWidgetComponent) return;

	if (IsLocallyControlledByPlayer())
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);
	}

	UMAOverHeadStatsGauge* OverheadStatsGuage = Cast<UMAOverHeadStatsGauge>(OverHeadWidgetComponent->GetUserWidgetObject());
	if (OverheadStatsGuage)
	{
		OverheadStatsGuage->ConfigureWithASC(GetAbilitySystemComponent());
		if (!IsLocallyControlledByPlayer()) // 자기 자신이 아닐 때만 표시
		{
			OverHeadWidgetComponent->SetHiddenInGame(false);
		}
		GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityUpdateTimerHandle);
		GetWorldTimerManager().SetTimer(HeadStatGaugeVisibilityUpdateTimerHandle, this, &AMACharacter::UpdateHeadGaugeVisibility, HeadStatGaugeVisibilityCheckUpdateGap, true);
	}
}

void AMACharacter::UpdateHeadGaugeVisibility()
{
	APawn* LocalPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (LocalPlayerPawn)
	{
		// 자기 자신이면 항상 숨김
		if (LocalPlayerPawn == this)
		{
			OverHeadWidgetComponent->SetHiddenInGame(true);
			return;
		}

		// 상대방일 경우 거리 기반 표시
		float DistSquared = FVector::DistSquared(GetActorLocation(), LocalPlayerPawn->GetActorLocation());
		OverHeadWidgetComponent->SetHiddenInGame(DistSquared > HeadStatGaugeVisibilityRangeSquared);
	}
}

void AMACharacter::Server_SetMaterialParams_Implementation(const FMaterialParamData& BodyData,
                                                           const FMaterialParamData& EyeData)
{
	if (LoadoutComponent)
	{
		LoadoutComponent->SetMaterialParams(BodyData, EyeData);
	}
}



/*************************************************************/
/*								Skill						 */
/*************************************************************/

void AMACharacter::Multicast_PlayNiagara_Implementation(UNiagaraSystem* NS, FTransform SpawnTransform, bool bApplyColor, FLinearColor EffectColor)
{
	if (GetNetMode() == NM_DedicatedServer)
            return;
	
	UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(), NS, SpawnTransform.GetLocation(), SpawnTransform.Rotator(), SpawnTransform.GetScale3D(), true);
	if (SpawnedVFX && bApplyColor)
	{
		SpawnedVFX->SetVariableLinearColor(FName("EffectColor"),EffectColor);
	}
}

void AMACharacter::Multicast_PlayNiagaraAttached_Implementation(UNiagaraSystem* NS, FName SocketName, FVector LocOffset,
	FRotator RotOffset, FVector Scale, bool bAutoDestroy, bool bApplyColor, FLinearColor EffectColor)
{
	if (GetNetMode() == NM_DedicatedServer)
            return;
	
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp) return;

	UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NS,GetMesh(),SocketName,LocOffset,RotOffset,
			Scale,EAttachLocation::KeepRelativeOffset,bAutoDestroy, 
			ENCPoolMethod::None,true);
	if (SpawnedVFX && bApplyColor)
	{
		SpawnedVFX->SetVariableLinearColor(FName("EffectColor"),EffectColor);
	}
}

void AMACharacter::Multicast_JumpToSection_Implementation(UAnimMontage* Montage, FName SectionName)
{
	if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
	{
		if (Montage && AnimInst->Montage_IsPlaying(Montage))
		{
			AnimInst->Montage_JumpToSection(SectionName, Montage);
		}
	}
}

bool AMACharacter::GetReactionAnimConfig(const FGameplayTag& ReactionTag, FReactionAnimConfig& OutConfig) const
{
	return ReactionComponent ? ReactionComponent->GetReactionAnimConfig(ReactionTag, OutConfig) : false;
}
