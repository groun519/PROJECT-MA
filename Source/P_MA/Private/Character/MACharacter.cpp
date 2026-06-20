#include "Character/MACharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/MAElementalComponent.h"
#include "Character/MAImpulseComponent.h"
#include "Character/MAOverlayComponent.h"
#include "Character/MAStatusEffectComponent.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Area/Decal/MASkillAreaDecalStatics.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
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
	MAAbilitySystemComponent->AddAttributeSetSubobject(MAAttributeSet.Get());
	StatusEffectComponent = CreateDefaultSubobject<UMAStatusEffectComponent>("Reaction Component");
	ElementalComponent = CreateDefaultSubobject<UMAElementalComponent>("Elemental Component");
	ImpulseComponent = CreateDefaultSubobject<UMAImpulseComponent>("Impulse Component");
	OverlayComponent = CreateDefaultSubobject<UMAOverlayComponent>("Overlay Component");
	OverHeadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("Over Head Widget Component");
	OverHeadWidgetComponent->SetupAttachment(GetMesh());
	LoadoutComponent = CreateDefaultSubobject<ULoadoutComponent>("LoadoutComponent");
	SkillManagerComponent = CreateDefaultSubobject<UMASkillManagerComponent>("SkillManagerComponent");

	BindGASChangeDelegates();

	PerceptionStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("Perception Stimuli Source Component");
}

void AMACharacter::ServerSideInit()
{
	MAAbilitySystemComponent->InitAbilityActorInfo(this, this);
	MAAbilitySystemComponent->ServerSideInit();
	if (SkillManagerComponent)
	{
		SkillManagerComponent->InitializeGrantedAbilities();
	}
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

void AMACharacter::BeginPlay()
{
	Super::BeginPlay();
	ConfigureOverHeadStatusWidget();

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
	FGameplayEventData LocalEventData = EventData;
	LocalEventData.EventTag = EventTag;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventTag, LocalEventData);
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
		MAAbilitySystemComponent->RegisterGameplayTagEvent(UMAAbilitySystemStatics::GetMoveBlockTag()).AddUObject(this, &AMACharacter::MoveBlockTagUpdated);
		MAAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetMoveSpeedAttribute()).AddUObject(this, &AMACharacter::MoveSpeedUpdated);
		MAAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetSlowMultiplierAttribute()).AddUObject(this, &AMACharacter::MoveSpeedUpdated);
	}
}

void AMACharacter::DeathTagUpdated(const FGameplayTag /*Tag*/, int32 NewCount)
{
	if (NewCount != 0) StartDeathSequence();
	else GetWorldTimerManager().SetTimerForNextTick(this, &AMACharacter::Respawn);
}

void AMACharacter::MoveBlockTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (IsDead()) return;
	if (NewCount != 0)
	{
		if (ImpulseComponent)
			ImpulseComponent->CancelInterruptibleActionImpulses();

		StopMovementForBlock();
	}
	RefreshMaxWalkSpeed();
}

void AMACharacter::MoveSpeedUpdated(const FOnAttributeChangeData& /*Data*/)
{
	RefreshMaxWalkSpeed();
}

void AMACharacter::SetStatusGaugeEnabled(bool bIsEnabled)
{
	bStatusGaugeEnabled = bIsEnabled;

	if (!OverHeadWidgetComponent) return;

	OverHeadWidgetComponent->SetHiddenInGame(!bStatusGaugeEnabled);
	if (bStatusGaugeEnabled)
	{
		if (UMAOverHeadStatsGauge* OverheadStatsGuage = EnsureOverHeadStatusWidgetConfigured())
		{
			OverheadStatsGuage->RefreshStatusEffectDisplay();
		}
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

	const float MoveSpeed = MAAttributeSet->GetMoveSpeed() * MAAttributeSet->GetSlowMultiplier();
	const float MovementResponsiveness = 10.f;
	if (MAAbilitySystemComponent->HasMatchingGameplayTag(UMAAbilitySystemStatics::GetMoveBlockTag()))
	{
		MoveComp->MaxWalkSpeed = 0.f;
		MoveComp->MaxAcceleration = 0.f;
		MoveComp->BrakingDecelerationWalking = 0.f;
		return;
	}

	MoveComp->MaxWalkSpeed = MoveSpeed;
	MoveComp->MaxAcceleration = MoveSpeed * MovementResponsiveness;
	MoveComp->BrakingDecelerationWalking = MoveSpeed * MovementResponsiveness;
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

void AMACharacter::StartDeathSequence()
{
	OnDead();
	if (StatusEffectComponent) StatusEffectComponent->ResetTransientStatusEffectState();
	if (MAAbilitySystemComponent) MAAbilitySystemComponent->CancelAllAbilities();
	
	if (DeathMontage) PlayAnimMontage(DeathMontage);
	SetStatusGaugeEnabled(false);

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetAIPerceptionStimuliSourceEnabled(false);
}

void AMACharacter::Respawn()
{
	if (StatusEffectComponent) StatusEffectComponent->ResetTransientStatusEffectState();

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
		MAAbilitySystemComponent->ApplyReviveStatEffect();
	}

	RefreshMaxWalkSpeed();
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

	OverHeadWidgetComponent->SetHiddenInGame(!bStatusGaugeEnabled);
	EnsureOverHeadStatusWidgetConfigured();
}

UMAOverHeadStatsGauge* AMACharacter::EnsureOverHeadStatusWidgetConfigured()
{
	if (!OverHeadWidgetComponent) return nullptr;

	UMAOverHeadStatsGauge* OverheadStatsGuage = Cast<UMAOverHeadStatsGauge>(OverHeadWidgetComponent->GetUserWidgetObject());
	if (!OverheadStatsGuage) return nullptr;

	OverheadStatsGuage->InitializeFromCharacter(this);
	return OverheadStatsGuage;
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

void AMACharacter::Multicast_AttachNiagaraToSelf_Implementation(UNiagaraSystem* NS, FName SocketName, float LifeSpan)
{
	if (GetNetMode() == NM_DedicatedServer || !NS) return;

	USceneComponent* AttachComponent = GetMesh() ? static_cast<USceneComponent*>(GetMesh()) : GetRootComponent();
	if (!AttachComponent) return;

	UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
		NS,
		AttachComponent,
		SocketName,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		FVector::OneVector,
		EAttachLocation::SnapToTarget,
		true,
		ENCPoolMethod::None,
		true);
	if (!SpawnedVFX || LifeSpan <= 0.f) return;

	FTimerHandle DestroyTimerHandle;
	TWeakObjectPtr<UNiagaraComponent> WeakVFX = SpawnedVFX;
	GetWorldTimerManager().SetTimer(
		DestroyTimerHandle,
		[WeakVFX]()
		{
			if (UNiagaraComponent* VFX = WeakVFX.Get())
			{
				VFX->DestroyComponent();
			}
		},
		LifeSpan,
		false);
}

void AMACharacter::Multicast_SpawnSkillAreaImpact_Implementation(FMASkillWorldAreaShape Area)
{
	if (GetNetMode() == NM_DedicatedServer) return;

	MASkillAreaDecalStatics::SpawnImpact(this, nullptr, Area);
}

/** Status Effect **/
void AMACharacter::Multicast_PlayStatusEffectImpulse_Implementation(const FGameplayTag& StatusEffectTag, float Magnitude, FVector SourcePoint)
{
	if (HasAuthority()) return;
	if (!StatusEffectComponent) return;

	StatusEffectComponent->PlayReplicatedStatusEffectImpulse(StatusEffectTag, Magnitude, SourcePoint);
}

bool AMACharacter::GetStatusEffectAnimConfig(const FGameplayTag& StatusEffectTag, FStatusEffectAnimConfig& OutConfig) const
{
	return StatusEffectComponent ? StatusEffectComponent->GetStatusEffectAnimConfig(StatusEffectTag, OutConfig) : false;
}
