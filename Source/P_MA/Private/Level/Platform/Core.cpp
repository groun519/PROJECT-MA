// Fill out your copyright notice in the Description page of Project Settings.

#include "Core.h"
#include "Convenience/InteractComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"

ACore::ACore()
{
	PrimaryActorTick.bCanEverTick = true;

	InteractComp = CreateDefaultSubobject<UInteractComponent>("InteractRangeSphere");
	InteractComp->SetupAttachment(GetRootComponent());

	OuterCoreMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("OuterCoreMesh"));
	OuterCoreMesh->SetupAttachment(GetMesh());
}

void ACore::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		SetGenericTeamId(FGenericTeamId(0));
	}
	if (InteractComp)
	{
		// 사용예시. 매크로로 함수 쉽게 넘김.
		InteractComp->CALL_SETUP_INTERACT(HandleInteract);
	}

	DynamicMaterials.Reset();
	if (USkeletalMeshComponent* InnerMesh = GetMesh())
	{
		const int32 MatCount = InnerMesh->GetNumMaterials();
		for (int32 MatIndex = 0; MatIndex < MatCount; ++MatIndex)
		{
			if (UMaterialInstanceDynamic* NewDynMat = InnerMesh->CreateDynamicMaterialInstance(MatIndex))
			{
				DynamicMaterials.Add(NewDynMat);
			}
		}
	}

	if (OuterCoreMesh)
	{
		const int32 MatCount = OuterCoreMesh->GetNumMaterials();
		for (int32 MatIndex = 0; MatIndex < MatCount; ++MatIndex)
		{
			if (UMaterialInstanceDynamic* NewDynMat = OuterCoreMesh->CreateDynamicMaterialInstance(MatIndex))
			{
				DynamicMaterials.Add(NewDynMat);
			}
		}
	}

	if (USceneComponent* RootComp = GetRootComponent())
	{
		BaseRelativeZ = RootComp->GetRelativeLocation().Z;
		BaseRelativeRotation = RootComp->GetRelativeRotation();
	}
	CurrentColor = BaseColor;
	SpinYaw = BaseRelativeRotation.Yaw;
	ApplyCurrentColor();
}

void ACore::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 컬러
	if (bColorInterpActive)
	{
		ColorInterpElapsed += DeltaTime;
		const float Alpha = FMath::Clamp(
			ColorInterpElapsed / FMath::Max(ColorInterpDuration, KINDA_SMALL_NUMBER),
			0.f,
			1.f
		);
		CurrentColor = FLinearColor::LerpUsingHSV(StartColor, TargetColor, Alpha);
		ApplyCurrentColor();

		if (Alpha >= 1.f)
		{
			bColorInterpActive = false;
			CurrentColor = TargetColor;
		}
	}

	if (HasAuthority())
	{
		// 위아래 움직임
		if (MoveSetting.bUseBobMove && MoveSetting.BobAmplitude > 0.f)
		{
			BobTime += DeltaTime * MoveSetting.BobSpeed;
			const float OffsetZ = FMath::Sin(BobTime) * MoveSetting.BobAmplitude;
			FVector NewLoc = FVector::ZeroVector;
			NewLoc.Z = BaseRelativeZ + OffsetZ;
			SetActorRelativeLocation(NewLoc);
		}

		// 회전
		if (MoveSetting.bUseSpin && MoveSetting.SpinYawSpeed != 0.f)
		{
			FRotator NewRot = BaseRelativeRotation;
			SpinYaw += MoveSetting.SpinYawSpeed * DeltaTime;
			SpinYaw = FMath::Fmod(SpinYaw, 360.f);
			if (SpinYaw < 0.f) SpinYaw += 360.f;
			NewRot.Yaw += SpinYaw;
			SetActorRelativeRotation(NewRot);
		}
	}
}

void ACore::HandleInteract(AMAPlayerCharacter* Interactor)
{
	UE_LOG(LogTemp, Display, TEXT("Core Interacted!"));
	// 여기에 추가해주면 됩니다 용범BROTHER 
}

void ACore::ApplyBattleColor(bool bInBattle)
{
	if (HasAuthority())
	{
		const FLinearColor NewTarget = bInBattle ? BattleColor : BaseColor;
		ReplicatedTargetColor = NewTarget;
		StartColorInterp(NewTarget);
		return;
	}

	Server_ApplyBattleColor(bInBattle);
}

void ACore::Server_ApplyBattleColor_Implementation(bool bInBattle)
{
	ApplyBattleColor(bInBattle);
}

void ACore::ApplyCurrentColor()
{
	if (DynamicMaterials.IsEmpty()) return;

	for (UMaterialInstanceDynamic* DynMatPtr : DynamicMaterials)
	{
		if (!DynMatPtr) continue;
		DynMatPtr->SetVectorParameterValue(ColorParamName, CurrentColor);
	}
}

void ACore::StartColorInterp(const FLinearColor& NewTarget)
{
	TargetColor = NewTarget;
	if (TargetColor == CurrentColor) return;

	StartColor = CurrentColor;
	ColorInterpElapsed = 0.f;
	bColorInterpActive = true;
}

void ACore::OnRep_TargetColor()
{
	StartColorInterp(ReplicatedTargetColor);
}

void ACore::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACore, ReplicatedTargetColor);
}
