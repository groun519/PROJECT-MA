#include "Player/Revive/MAReviveActor.h"

#include "Components/DecalComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MAMaterialParams.h"
#include "Net/UnrealNetwork.h"
#include "P_MA/P_MA.h"
#include "Player/MAPlayerCharacter.h"

AMAReviveActor::AMAReviveActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	ReviveArea = CreateDefaultSubobject<USphereComponent>(TEXT("ReviveArea"));
	ReviveArea->SetupAttachment(Root);
	ReviveArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ReviveArea->SetCollisionResponseToAllChannels(ECR_Ignore);
	ReviveArea->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Overlap);
	ReviveArea->SetGenerateOverlapEvents(true);

	ReviveDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("ReviveDecal"));
	ReviveDecal->SetupAttachment(Root);
	ReviveDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	ReviveDecal->SortOrder = 1;

	ReviveBackgroundDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("ReviveBackgroundDecal"));
	ReviveBackgroundDecal->SetupAttachment(Root);
	ReviveBackgroundDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	ReviveBackgroundDecal->SortOrder = 0;
}

void AMAReviveActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (ReviveArea)
	{
		ReviveArea->SetSphereRadius(ReviveRadius);
	}
	if (ReviveDecal)
	{
		ReviveDecal->SetRelativeLocation(FVector(0.f, 0.f, -DecalProjectionDepth + 10.f));
		ReviveDecal->DecalSize = FVector(DecalProjectionDepth, ReviveRadius, ReviveRadius);
	}
	if (ReviveBackgroundDecal)
	{
		ReviveBackgroundDecal->SetRelativeLocation(FVector(0.f, 0.f, -DecalProjectionDepth + 10.f));
		ReviveBackgroundDecal->DecalSize = FVector(DecalProjectionDepth, ReviveRadius, ReviveRadius);
	}
	RefreshReviveDecalVisual();
	RefreshReviveDecalProgress();
}

void AMAReviveActor::BeginPlay()
{
	Super::BeginPlay();

	RefreshReviveDecalVisual();
	RefreshReviveDecalProgress();
}

void AMAReviveActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMAReviveActor, CurrentReviveProgress);
}

void AMAReviveActor::InitializeReviveTarget(AMAPlayerCharacter* InTargetPlayer)
{
	TargetPlayer = InTargetPlayer;
}

void AMAReviveActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!HasAuthority()) return;

	AMAPlayerCharacter* Target = TargetPlayer.Get();
	if (!Target || !Target->IsDead())
	{
		Destroy();
		return;
	}

	const int32 ReviverCount = CountRevivers();
	if (ReviverCount <= 0) return;

	CurrentReviveProgress += 0.1f * ReviverCount * DeltaSeconds;
	RefreshReviveDecalProgress();
	if (CurrentReviveProgress >= 1.0f)
	{
		CompleteRevive();
	}
}

void AMAReviveActor::OnRep_ReviveProgress()
{
	RefreshReviveDecalProgress();
}

int32 AMAReviveActor::CountRevivers() const
{
	if (!ReviveArea) return 0;

	TArray<AActor*> OverlappingActors;
	ReviveArea->GetOverlappingActors(OverlappingActors, AMAPlayerCharacter::StaticClass());

	int32 Count = 0;
	const AMAPlayerCharacter* Target = TargetPlayer.Get();
	for (AActor* Actor : OverlappingActors)
	{
		const AMAPlayerCharacter* Player = Cast<AMAPlayerCharacter>(Actor);
		if (!Player || Player == Target || Player->IsDead()) continue;

		++Count;
	}
	return Count;
}

void AMAReviveActor::CompleteRevive()
{
	AMAPlayerCharacter* Target = TargetPlayer.Get();
	if (Target && Target->IsDead())
	{
		Target->RespawnImmediately();
	}
	Destroy();
}

void AMAReviveActor::RefreshReviveDecalVisual()
{
	if (ReviveDecal && !ReviveDecalMID)
	{
		if (ReviveDecalMaterial)
		{
			ReviveDecal->SetDecalMaterial(ReviveDecalMaterial);
		}

		ReviveDecalMID = ReviveDecal->CreateDynamicMaterialInstance();
	}
	if (ReviveBackgroundDecal && !ReviveBackgroundDecalMID)
	{
		if (ReviveDecalMaterial)
		{
			ReviveBackgroundDecal->SetDecalMaterial(ReviveDecalMaterial);
		}

		ReviveBackgroundDecalMID = ReviveBackgroundDecal->CreateDynamicMaterialInstance();
	}

	if (ReviveDecalMID)
	{
		ReviveDecalMID->SetScalarParameterValue(PARAM_AreaDecal_BaseAngle, 0.f);
		ReviveDecalMID->SetVectorParameterValue(PARAM_AreaDecal_BaseColor, ReviveDecalColor);
	}
	if (ReviveBackgroundDecalMID)
	{
		ReviveBackgroundDecalMID->SetScalarParameterValue(PARAM_AreaDecal_BaseAngle, 0.f);
		ReviveBackgroundDecalMID->SetScalarParameterValue(PARAM_AreaDecal_EndAngle, 360.f);
		ReviveBackgroundDecalMID->SetVectorParameterValue(PARAM_AreaDecal_BaseColor, ReviveBackgroundDecalColor);
		ReviveBackgroundDecalMID->SetScalarParameterValue(PARAM_AreaDecal_Opacity, DecalOpacity);
	}
}

void AMAReviveActor::RefreshReviveDecalProgress()
{
	if (!ReviveDecalMID)
	{
		RefreshReviveDecalVisual();
	}
	if (!ReviveDecalMID) return;

	const float ProgressRatio = FMath::Clamp(CurrentReviveProgress, 0.f, 1.f);

	ReviveDecalMID->SetScalarParameterValue(PARAM_AreaDecal_Opacity, ProgressRatio > 0.f ? DecalOpacity : 0.f);
	ReviveDecalMID->SetScalarParameterValue(PARAM_AreaDecal_EndAngle, ProgressRatio * 360.f);
}
