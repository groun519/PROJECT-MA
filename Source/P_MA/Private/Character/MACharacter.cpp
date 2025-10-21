// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/MACharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MABaseProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Widget/MAOverHeadStatsGauge.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "P_MA/P_MA.h"

AMACharacter::AMACharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Target, ECR_Ignore);

	MAAbilitySystemComponent = CreateDefaultSubobject<UMAAbilitySystemComponent>("MAAbility System Component");
	MAAttributeSet = CreateDefaultSubobject<UMAAttributeSet>("MAAttribute Set");
	OverHeadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("Over Head Widget Component");
	OverHeadWidgetComponent->SetupAttachment(GetRootComponent());

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
	DOREPLIFETIME(AMACharacter, MaterialParamValue);
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

	if (GetMesh())
	{
		DynMat = GetMesh()->CreateAndSetMaterialInstanceDynamic(0);
		if (DynMat)
		{
			// Body Param Update
			//DynMat->SetScalarParameterValue("Body_Opacity",	BaseMaterialParam.BodyData.Opacity);
			DynMat->SetVectorParameterValue("Body_Color",	BaseMaterialParam.BodyData.Color);
			DynMat->SetScalarParameterValue("Body_Emissive",BaseMaterialParam.BodyData.Emissive);

			// Eye Param Update
			//DynMat->SetScalarParameterValue("Eye_Opacity",	BaseMaterialParam.EyeData.Opacity);
			DynMat->SetVectorParameterValue("Eye_Color",	BaseMaterialParam.EyeData.Color);
			DynMat->SetScalarParameterValue("Eye_Emissive", BaseMaterialParam.EyeData.Emissive);
		}
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
	//Aim태그 변경시 -> 이동 속도 느리게
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

/** Mat System Section **//**
 *	머티리얼 파라미터 변경하는 섹션
 */
void AMACharacter::OnRep_MaterialParam()
{
	ApplyMaterialParam();
}

void AMACharacter::ApplyMaterialParam()
{
	if (DynMat)
	{
		// Body Param Update
		//DynMat->SetScalarParameterValue("Body_Opacity", MaterialParamValue.BodyData.Opacity);
		DynMat->SetVectorParameterValue("Body_Color", MaterialParamValue.BodyData.Color);
		DynMat->SetScalarParameterValue("Body_Emissive", MaterialParamValue.BodyData.Emissive);

		// Eye Param Update
		//DynMat->SetScalarParameterValue("Eye_Opacity", MaterialParamValue.EyeData.Opacity);
		DynMat->SetVectorParameterValue("Eye_Color", MaterialParamValue.EyeData.Color);
		DynMat->SetScalarParameterValue("Eye_Emissive", MaterialParamValue.EyeData.Emissive);
	}
}

void AMACharacter::Server_SetMaterialParams_Implementation(const FMaterialParamData& BodyData,
                                                           const FMaterialParamData& EyeData)
{
	MaterialParamValue.BodyData = BodyData;
	MaterialParamValue.EyeData  = EyeData;

	ApplyMaterialParam();
}



UNiagaraComponent* AMACharacter::GetWeaponEffectComponent() const
{
	return nullptr;
}

void AMACharacter::ActivateWeaponEffect(UNiagaraSystem* Effect)
{
}

void AMACharacter::DeactivateWeaponEffect()
{
}


/*************************************************************/
/*								Skill						 */
/*************************************************************/

void AMACharacter::Server_SpawnProjectile_Implementation(TSubclassOf<class AMABaseProjectile> ProjectileClass,
	FVector Location, FRotator Rotation,float CollisionRadius)
{
	UWorld* World = this->GetWorld();
	if (World && ProjectileClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AMABaseProjectile* SpawnedProjectile =World->SpawnActor<AMABaseProjectile>(ProjectileClass,Location, Rotation, SpawnParams);

		if (SpawnedProjectile && CollisionRadius > 0.f)
		{
			if (USphereComponent* SphereComponent = SpawnedProjectile->CollisionComponent)
			{
				SphereComponent->SetSphereRadius(CollisionRadius);
			}
		}
	}
}
