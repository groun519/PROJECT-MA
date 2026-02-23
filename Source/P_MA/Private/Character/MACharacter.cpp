// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/MACharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AIController.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
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

AMACharacter::AMACharacter()
{
	PrimaryActorTick.bCanEverTick = true;

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
	OverHeadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("Over Head Widget Component");
	OverHeadWidgetComponent->SetupAttachment(GetRootComponent());
	LoadoutComponent = CreateDefaultSubobject<ULoadoutComponent>("LoadoutComponent");

	BindGASChangeDelegates();

	PerceptionStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("Perception Stimuli Source Component");
}

void AMACharacter::ServerSideInit()
{
	MAAbilitySystemComponent->InitAbilityActorInfo(this, this);
	//MAAbilitySystemComponent->ApplyInitialEffects();
	//MAAbilitySystemComponent->GiveInitialAbilities();
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

void AMACharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
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
		MAAbilitySystemComponent->RegisterGameplayTagEvent(UMAAbilitySystemStatics::GetStunStatTag()).AddUObject(this, &AMACharacter::StunTagUpdated);
		MAAbilitySystemComponent->RegisterGameplayTagEvent(UMAAbilitySystemStatics::GetAimingTag()).AddUObject(this, &AMACharacter::AimTagUpdated);
		MAAbilitySystemComponent->RegisterGameplayTagEvent(UMAAbilitySystemStatics::GetMoveBlockTag()).AddUObject(this, &AMACharacter::MoveBlockTagUpdated);
		MAAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetMoveSpeedAttribute()).AddUObject(this, &AMACharacter::MoveSpeedUpdated);
		MAAbilitySystemComponent->AddGameplayEventTagContainerDelegate(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(TEXT("Stats.Knockdown"))),FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &AMACharacter::OnKnockdownEvent));
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

void AMACharacter::StunTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (IsDead()) return;
	if (NewCount != 0)
	{
		OnStun();
		PlayAnimMontage(StunMontage);
	}
	else
	{
		OnRecoverFromStun();
		StopAnimMontage(StunMontage);
	}
	
}

void AMACharacter::AimTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (IsDead()) return;
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp) return;
	
	const float MoveSpeed = MAAttributeSet->GetMoveSpeed();
	if (NewCount != 0)
	{
		MoveComp->MaxWalkSpeed = MoveSpeed*0.2;
	}
	else
	{
		MoveComp->MaxWalkSpeed = MoveSpeed;
	}
}

void AMACharacter::MoveBlockTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (IsDead()) return;
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp) return;
	const float MoveSpeed = MAAttributeSet->GetMoveSpeed();
	if (NewCount != 0)
	{
		MoveComp->MaxWalkSpeed =0.f;
	}
	else
	{
		MoveComp->MaxWalkSpeed = MoveSpeed;
	}
}

void AMACharacter::MoveSpeedUpdated(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}

void AMACharacter::SetStatusGaugeEnabled(bool bIsEnabled)
{
	GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityUpdateTimerHandle);
	if (bIsEnabled)
	{
		// TODO:
		// ConfigureOverHeadStatusWidget();
	}
	else
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);
	}
}

void AMACharacter::OnStun()
{
}

void AMACharacter::OnRecoverFromStun()
{
}

bool AMACharacter::IsDead() const
{
	return GetAbilitySystemComponent() -> HasMatchingGameplayTag(UMAAbilitySystemStatics::GetDeadStatTag());
}

void AMACharacter::RespawnImmediately()
{
	if (HasAuthority())
		GetAbilitySystemComponent() -> RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(UMAAbilitySystemStatics::GetDeadStatTag()));
}

// void AMACharacter::DeathMontageFinished()
// {
// 	SetRagdollEnabled(true);
// }

// void AMACharacter::SetRagdollEnabled(bool bIsEnabled)
// {
// 	if (bIsEnabled)
// 	{
// 		GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
// 		GetMesh()->SetSimulatePhysics(true);
// 		GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
// 	}
// 	else
// 	{
// 		GetMesh()->SetSimulatePhysics(false);
// 		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
// 		GetMesh()->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
// 		GetMesh()->SetRelativeTransform(MeshRelativeTransform);
// 	}
// }

void AMACharacter::PlayDeathAnimation()
{
	if (DeathMontage)
	{
		float MontageDuration = PlayAnimMontage(DeathMontage);
		//GetWorldTimerManager().SetTimer(DeathMontageTimerHandle, this, &AMACharacter::DeathMontageFinished, MontageDuration + DeathMontageFinishTimeShift);
	}
}

void AMACharacter::StartDeathSequence()
{
	OnDead();

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
	OnRespawn();
	SetAIPerceptionStimuliSourceEnabled(true);
	//SetRagdollEnabled(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetMesh()->GetAnimInstance()->StopAllMontages(0.f);
	SetStatusGaugeEnabled(true);

	if (HasAuthority() && GetController())
	{
		TWeakObjectPtr<AActor> StartSpot = GetController()->StartSpot;
		if (StartSpot.IsValid())
		{ 
			SetActorTransform(StartSpot->GetActorTransform());
		}
	}

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

void AMACharacter::OnRep_TeamID()
{
	// override only
}

void AMACharacter::SetAIPerceptionStimuliSourceEnabled(bool bIsEnabled)
{
	if (!PerceptionStimuliSourceComponent)		return;

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
	if (!OverHeadWidgetComponent)
	{
		return; 
	}

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

void AMACharacter::Multicast_PlayFlinchMontage_Implementation(FName SectionName)
{
	if (IsDead() || bPendingKnockdown)
	{
		return;
	}
	if (MAAbilitySystemComponent && MAAbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Stats.Immunity.Flinch")))
	{
		return;
	}
	if (FlinchMontage)
	{
		if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
		{
			AnimInst->Montage_Play(FlinchMontage);
			AnimInst->Montage_JumpToSection(SectionName, FlinchMontage);
		}
	}
}

void AMACharacter::Server_ApplyFlinch(AActor* Attacker)
{
	if (!HasAuthority() || IsDead() || bPendingKnockdown)
		return;
	
	FGameplayTag FlinchImmunityTag = FGameplayTag::RequestGameplayTag("State.Immune.Flinch");
	if (MAAbilitySystemComponent && !MAAbilitySystemComponent->HasMatchingGameplayTag(FlinchImmunityTag))
	{
		FGameplayTagContainer CancelTags;
		CancelTags.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
		MAAbilitySystemComponent->CancelAbilities(&CancelTags);

		FGameplayTag FlinchingTag = FGameplayTag::RequestGameplayTag("State.Debuff.Flinch");
		MAAbilitySystemComponent->AddLooseGameplayTag(FlinchingTag);

		if (AAIController* AIC = Cast<AAIController>(GetController()))
		{
			if (UBlackboardComponent* BBC = AIC->GetBlackboardComponent())
			{
				BBC->SetValueAsBool(FName("IsFlinching"),true);
			}
		}
		FName SectionName = FName("Front");
		float MontageLength = 1.f;

		if (Attacker && FlinchMontage)
		{
			FVector DirToAttacker = (Attacker->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			DirToAttacker.Z = 0.f;

			FVector MyForward = GetActorForwardVector();
			FVector MyRight = GetActorRightVector();

			float ForwardDot = FVector::DotProduct(DirToAttacker, MyForward);
			float RightDot = FVector::DotProduct(DirToAttacker, MyRight);
			
			if (ForwardDot >= 0.5f)
			{	//앞피격
				SectionName = FName("Front");
				UE_LOG(LogTemp,Warning,TEXT("Section Name == Front"));
			}
			else if (ForwardDot <= -0.5f)
			{	//뒤피격
				SectionName = FName("Back");
				UE_LOG(LogTemp,Warning,TEXT("Section Name == Back"));
			}
			else if (RightDot >= 0.5f)
			{	//우피격
				SectionName = FName("Right");
				UE_LOG(LogTemp,Warning,TEXT("Section Name == Right"));
			}
			else if (RightDot <= -0.5f)
			{	//좌피격
				SectionName = FName("Left");
				UE_LOG(LogTemp,Warning,TEXT("Section Name == Left"));
			}

			int32 SectionIndex = FlinchMontage->GetSectionIndex(SectionName);
			if (SectionIndex != INDEX_NONE)
			{
				MontageLength = FlinchMontage->GetSectionLength(SectionIndex);
			}
		}
		
		Multicast_PlayFlinchMontage(SectionName);

		FTimerHandle FlinchTimerHandle;
		GetWorldTimerManager().SetTimer(FlinchTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this, FlinchingTag]()
		{
			if (MAAbilitySystemComponent)
			{
				MAAbilitySystemComponent->RemoveLooseGameplayTag(FlinchingTag);
			}
			
			if (AAIController* AIC = Cast<AAIController>(GetController()))
			{
				if (UBlackboardComponent* BBC = AIC->GetBlackboardComponent())
				{
					BBC->SetValueAsBool(FName("IsFlinching"),false);
				}
			}
		}), MontageLength, false);
	}
}

void AMACharacter::Server_ApplyHitReaction(FGameplayTag ReactionTag, float Force, AActor* Attacker)
{
	if (!HasAuthority() || IsDead() || bPendingKnockdown)
		return;

	//스턴 처리
	if (ReactionTag == FGameplayTag::RequestGameplayTag("Effect.Reaction.Stun"))
	{
		if (MAAbilitySystemComponent && !MAAbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("State.Immune.Stun")))
		{
			MAAbilitySystemComponent->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag("State.Debuff.Stun"));
			UE_LOG(LogTemp,Warning,TEXT("스턴"));
		}
	}

	//위치 이동계 처리 (넉백, 에어본, 넉다운)
	bool bIsPushReaction = ReactionTag.MatchesTag(FGameplayTag::RequestGameplayTag("Effect.Reaction.Knockback")) ||
			ReactionTag.MatchesTag(FGameplayTag::RequestGameplayTag("Effect.Reaction.Airborne")) ||
			ReactionTag.MatchesTag(FGameplayTag::RequestGameplayTag("Effect.Reaction.Knockdown"));

	if (bIsPushReaction)
	{
		if (MAAbilitySystemComponent && MAAbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("State.Immune.Push")))
			return;

		FGameplayTagContainer CancelTags;
		CancelTags.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
		MAAbilitySystemComponent->CancelAbilities(&CancelTags);
		
		FVector PushDirection = FVector::ZeroVector;
		if (Attacker)
		{
			PushDirection = (GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal();
		}else
		{
			PushDirection = -GetActorForwardVector();
		}

		if (ReactionTag == FGameplayTag::RequestGameplayTag("Effect.Reaction.Knockback"))
		{
			UE_LOG(LogTemp,Warning,TEXT("넉백"));
			PushDirection.Z = 0.2f;
			LaunchCharacter(PushDirection * Force,true, true);
			Multicast_PlayFlinchMontage(FName("Front"));
		}
		else if (ReactionTag == FGameplayTag::RequestGameplayTag("Effect.Reaction.Airborne"))
		{
			UE_LOG(LogTemp,Warning,TEXT("에어본"));
			PushDirection = FVector(0.f, 0.f, 1.f); // 위로 솟구침
			LaunchCharacter(PushDirection * Force, true, true);
			// TODO: 에어본 전용 몽타주 Multicast
		}
		else if (ReactionTag == FGameplayTag::RequestGameplayTag("Effect.Reaction.Knockdown"))
		{
			UE_LOG(LogTemp,Warning,TEXT("넉다운"));
			PushDirection.Z = 0.5f; 
			LaunchCharacter(PushDirection * Force, true, true);
			
			FGameplayEventData Payload;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, FGameplayTag::RequestGameplayTag("State.Debuff.Knockdown"), Payload);
		}

		// Behavior Tree 블랙보드 중단 (기존 로직 동일)
		if (AAIController* AIController = Cast<AAIController>(GetController()))
		{
			if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
				BlackboardComp->SetValueAsBool(FName("IsFlinching"), true);
		}
		// 블랙보드 중단 해제
		FTimerHandle PushTimerHandle;
		GetWorldTimerManager().SetTimer(PushTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (AAIController* AIC = Cast<AAIController>(GetController()))
			{
				if (UBlackboardComponent* BBC = AIC->GetBlackboardComponent())
				{
					BBC->SetValueAsBool(FName("IsFlinching"), false);
				}
			}
		}), 1.0f, false);
		return;
	}

	if (ReactionTag == FGameplayTag::RequestGameplayTag("Effect.Reaction.Flinch") || !ReactionTag.IsValid())
	{
		UE_LOG(LogTemp,Warning,TEXT("짧은 경직"));
		Server_ApplyFlinch(Attacker);
	}
}

void AMACharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (!bPendingKnockdown)
		return;

	bPendingKnockdown = false;

	if (!KnockdownMontage)
		return;

	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim)
		return;

	Anim->Montage_Play(KnockdownMontage);

	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &AMACharacter::OnKnockdownMontageBlendingOut);

	Anim->Montage_SetBlendingOutDelegate(BlendOutDelegate, KnockdownMontage);
}


void AMACharacter::OnKnockdownEvent(FGameplayTag EventTag, const FGameplayEventData* Payload)
{
	if (IsDead())
		return;

	bPendingKnockdown = true;

	if (MAAbilitySystemComponent)
	{
		MAAbilitySystemComponent->AddLooseGameplayTag(UMAAbilitySystemStatics::GetKnockdownTag());
	}
}

void AMACharacter::ResetKnockdownState()
{
	bPendingKnockdown = false;

	if (MAAbilitySystemComponent)
	{
		MAAbilitySystemComponent->RemoveLooseGameplayTag(
			UMAAbilitySystemStatics::GetKnockdownTag());
	}

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		if (Move->MovementMode == MOVE_None)
		{
			Move->SetMovementMode(MOVE_Walking);
		}
	}
}

void AMACharacter::OnKnockdownMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != KnockdownMontage)
		return;

	if (IsDead())
		return;

	ResetKnockdownState();
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
