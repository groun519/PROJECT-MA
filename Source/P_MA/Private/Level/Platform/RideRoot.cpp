// Fill out your copyright notice in the Description page of Project Settings.

#include "RideRoot.h"
#include "Components/TextRenderComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "P_MA/P_MA.h"
#include "EngineUtils.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/Components/ReadyStateComponent.h"
#include "Player/Components/ReadyRideComponent.h"

ARideRoot::ARideRoot()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	{
		FRepMovement RepMove = GetReplicatedMovement();
		RepMove.RotationQuantizationLevel = ERotatorQuantization::ShortComponents;
		SetReplicatedMovement(RepMove);
	}

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	/** Ready Text **/
	ReadyText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ReadyText"));
	ReadyText->SetupAttachment(RootComponent);
	ReadyText->SetHorizontalAlignment(EHTA_Center);
	ReadyText->SetVerticalAlignment(EVRTA_TextCenter);
	ReadyText->SetWorldSize(40.f);
	ReadyText->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
	ReadyText->SetText(FText::FromString(TEXT("[ 0 / 0 ]")));
	ReadyText->SetVisibility(false, true);

	RangeClampVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RangeClampVFX"));
	RangeClampVFX->SetupAttachment(RootComponent);
	RangeClampVFX->SetVisibility(false, true);
	RangeClampVFX->SetAutoActivate(false);

	MoveInTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("MoveInTrigger"));
	MoveInTrigger->SetupAttachment(RootComponent);
	MoveInTrigger->SetMobility(EComponentMobility::Movable);
	MoveInTrigger->SetNetAddressable();
	MoveInTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MoveInTrigger->SetCollisionObjectType(ECC_WorldDynamic);
	MoveInTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	MoveInTrigger->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Overlap);
	MoveInTrigger->SetGenerateOverlapEvents(true);

	ReadyRangeDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("ReadyRangeDecal"));
	ReadyRangeDecal->SetupAttachment(RootComponent);
	ReadyRangeDecal->SetRelativeLocation(FVector(0.f, 0.f, -100.f));
	ReadyRangeDecal->SetVisibility(false, true);
}

void ARideRoot::BeginPlay()
{
	Super::BeginPlay();

	if (MoveInTrigger)
	{
		MoveInTrigger->OnComponentBeginOverlap.AddDynamic(this, &ARideRoot::HandleMoveInTriggerBeginOverlap);
		MoveInTrigger->OnComponentEndOverlap.AddDynamic(this, &ARideRoot::HandleMoveInTriggerEndOverlap);
	}

	UpdateRangeClampVFXWorldLocation();
}

void ARideRoot::SetWaitMoveIn(bool bWaitMoveIn)
{
	if (HasAuthority())
	{
		bReadyTextVisible = bWaitMoveIn;
	}
	if (ReadyText)
	{
		ReadyText->SetVisibility(bWaitMoveIn, true);
	}
	if (ReadyRangeDecal)
	{
		ReadyRangeDecal->SetVisibility(bWaitMoveIn, true);
	}

	if (!HasAuthority() || !MoveInTrigger) return;

	if (bWaitMoveIn)
	{
		MoveInTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		MoveInTrigger->UpdateOverlaps();
		SyncReadyByMoveInTrigger(true);
	}
	else
	{
		MoveInTrigger->UpdateOverlaps();
		MoveInTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ARideRoot::ReleaseAttachedPlayers()
{
	if (!HasAuthority() || !GetWorld()) return;

	for (TActorIterator<AMAPlayerCharacter> It(GetWorld()); It; ++It)
	{
		AMAPlayerCharacter* PlayerCharacter = *It;
		if (!PlayerCharacter) continue;

		UReadyRideComponent* ReadyRideComp = PlayerCharacter->GetReadyRideComponent();
		if (!ReadyRideComp || ReadyRideComp->GetRidingRoot() != this) continue;

		if (PlayerCharacter->GetMovementBase() == GetRideBaseComponent())
		{
			PlayerCharacter->SetBase(nullptr);
		}

		ReadyRideComp->SetRidingRoot(nullptr);
	}
}

void ARideRoot::SetCurSpline(USplineComponent* Spline)
{
	if (CurSpline != Spline)
	{
		CurSpline = Spline;
		Distance = 0.f;
	}
}

void ARideRoot::SetReadyText(int32 ReadyCount, int32 TotalCount)
{
	if (HasAuthority())
	{
		ReplicatedReadyCounts = FIntPoint(ReadyCount, TotalCount);
	}
	
	if (!ReadyText) return;

	const FString NewText = FString::Printf(TEXT("[ %d / %d ]"), ReadyCount, TotalCount);
	ReadyText->SetText(FText::FromString(NewText));
}

void ARideRoot::SetRangeClampVisual(bool bVisible, float InSize)
{
	if (!HasAuthority()) return;

	const FRangeClampVisualState PrevState = AppliedRangeClampVisualState;
	if (PrevState.bVisible == bVisible && FMath::IsNearlyEqual(PrevState.Size, InSize)) return;

	ReplicatedRangeClampVisualState.bVisible = bVisible;
	ReplicatedRangeClampVisualState.Size = InSize;

	ApplyRangeClampVisual();

	if (RangeClampVFX && ReplicatedRangeClampVisualState.bVisible &&
		(!PrevState.bVisible || !FMath::IsNearlyEqual(PrevState.Size, ReplicatedRangeClampVisualState.Size)))
	{
		RangeClampVFX->ReinitializeSystem();
	}

	ForceNetUpdate();
}

void ARideRoot::OnRep_ReadyCounts()
{
	if (!ReadyText) return;
	const FString NewText = FString::Printf(TEXT("[ %d / %d ]"), ReplicatedReadyCounts.X, ReplicatedReadyCounts.Y);
	ReadyText->SetText(FText::FromString(NewText));
}

void ARideRoot::OnRep_ReadyTextVisible()
{
	if (!ReadyText) return;
	ReadyText->SetVisibility(bReadyTextVisible, true);
	if (ReadyRangeDecal)
	{
		ReadyRangeDecal->SetVisibility(bReadyTextVisible, true);
	}
}

void ARideRoot::OnRep_RangeClampVisual()
{
	const FRangeClampVisualState PrevState = AppliedRangeClampVisualState;
	ApplyRangeClampVisual();

	if (RangeClampVFX && ReplicatedRangeClampVisualState.bVisible &&
		(!PrevState.bVisible || !FMath::IsNearlyEqual(PrevState.Size, ReplicatedRangeClampVisualState.Size)))
	{
		RangeClampVFX->ReinitializeSystem();
	}
}

void ARideRoot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARideRoot, ReplicatedReadyCounts);
	DOREPLIFETIME(ARideRoot, bReadyTextVisible);
	DOREPLIFETIME(ARideRoot, ReplicatedRangeClampVisualState);
}

void ARideRoot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateRangeClampVFXWorldLocation();
	if (!HasAuthority()) return;

	if (!IsValid(CurSpline))
	{
		CurSpline = nullptr;
		return;
	}
	float Len = CurSpline->GetSplineLength();

	Distance += MoveSpeed * DeltaTime;

	if (Distance >= Len)
	{
		Distance -= Len;
		MoveEnd();
		if (!IsValid(CurSpline)) return;
	}

	FVector TargetLoc =
		CurSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

	FRotator TargetRot =
		CurSpline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

	TargetRot.Pitch = 0.f;
	TargetRot.Roll  = 0.f;

	const float RotationInterpSpeed = 1.0f; 
	const FRotator CurrentRot = GetActorRotation();
	const FRotator SmoothedRot =
		FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, RotationInterpSpeed);

	TargetLoc.Z = GetActorLocation().Z;

	SetActorLocation(TargetLoc);
	SetActorRotation(SmoothedRot);
	UpdateRangeClampVFXWorldLocation();
}

void ARideRoot::MoveEnd()
{
	OnPlatformReachedEnd.Broadcast();
}

void ARideRoot::ApplyRangeClampVisual()
{
	if (!RangeClampVFX) return;

	UpdateRangeClampVFXWorldLocation();
	FName RangeClampSizeParamName = TEXT("Size");
	RangeClampVFX->SetVariableFloat(RangeClampSizeParamName, ReplicatedRangeClampVisualState.Size);
	RangeClampVFX->SetVisibility(ReplicatedRangeClampVisualState.bVisible, true);

	if (ReplicatedRangeClampVisualState.bVisible)
	{
		if (!RangeClampVFX->IsActive())
		{
			RangeClampVFX->Activate(true);
		}
	}
	else
	{
		if (RangeClampVFX->IsActive())
		{
			RangeClampVFX->Deactivate();
		}
	}

	AppliedRangeClampVisualState = ReplicatedRangeClampVisualState;
}

void ARideRoot::UpdateRangeClampVFXWorldLocation()
{
	if (!RangeClampVFX) return;
	const FVector RootLoc = GetActorLocation();
	RangeClampVFX->SetWorldLocation(FVector(RootLoc.X, RootLoc.Y, -100.f));
}

void ARideRoot::SyncReadyByMoveInTrigger(bool bReady)
{
	if (!HasAuthority() || !MoveInTrigger) return;

	TArray<AActor*> OverlappedActors;
	MoveInTrigger->GetOverlappingActors(OverlappedActors, AMAPlayerCharacter::StaticClass());

	for (AActor* Actor : OverlappedActors)
	{
		AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(Actor);
		if (!PlayerCharacter) continue;

		UReadyStateComponent* ReadyComp = PlayerCharacter->GetReadyStateComponent();
		if (!ReadyComp) continue;

		ReadyComp->SetReady(bReady);

		if (bReady)
		{
			PlayerCharacter->SetBase(GetRideBaseComponent());
			PlayerCharacter->GetReadyRideComponent()->SetRidingRoot(this);
		}
		else
		{
			if (PlayerCharacter->GetMovementBase() == GetRideBaseComponent())
			{
				PlayerCharacter->SetBase(nullptr);
			}
			PlayerCharacter->GetReadyRideComponent()->SetRidingRoot(nullptr);
		}
	}
}

UPrimitiveComponent* ARideRoot::GetRideBaseComponent() const
{
	return MoveInTrigger;
}

void ARideRoot::HandleMoveInTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !bReadyTextVisible) return;

	AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(OtherActor);
	if (!PlayerCharacter) return;

	UReadyStateComponent* ReadyComp = PlayerCharacter->GetReadyStateComponent();
	if (!ReadyComp) return;

	ReadyComp->SetReady(true);

	PlayerCharacter->SetBase(GetRideBaseComponent());
	PlayerCharacter->GetReadyRideComponent()->SetRidingRoot(this);
}

void ARideRoot::HandleMoveInTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority() || !bReadyTextVisible) return;

	AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(OtherActor);
	if (!PlayerCharacter) return;
	if (MoveInTrigger && MoveInTrigger->IsOverlappingActor(PlayerCharacter)) return;

	UReadyStateComponent* ReadyComp = PlayerCharacter->GetReadyStateComponent();
	if (!ReadyComp) return;

	ReadyComp->SetReady(false);

	if (PlayerCharacter->GetMovementBase() == GetRideBaseComponent())
	{
		PlayerCharacter->SetBase(nullptr);
	}
	PlayerCharacter->GetReadyRideComponent()->SetRidingRoot(nullptr);
}
