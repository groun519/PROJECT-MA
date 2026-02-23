// Fill out your copyright notice in the Description page of Project Settings.

#include "Level/Sector/Spline/MirrorSplineSector.h"
#include "Level/Sector/Spline/SplineSector.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "PCGComponent.h"

AMirrorSplineSector::AMirrorSplineSector()
{
}

void AMirrorSplineSector::BeginPlay()
{
	Super::BeginPlay();
	if (PCGComponent)
	{
		PCGComponent->Cleanup();
		PCGComponent->SetComponentTickEnabled(false);
		PCGComponent->Deactivate();
	}
	RebindSourceDelegates();
	RebuildFromSource();
}

void AMirrorSplineSector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindSourceDelegates();
	Super::EndPlay(EndPlayReason);
}

void AMirrorSplineSector::SetRandomSeed(int32 MaxValue)
{
	// Mirror-triggered randomization always targets source index 0.
	if (SourceSectors.IsEmpty() || !SourceSectors[0])
	{
		RebuildFromSource();
		return;
	}

	ActiveSourceSector = SourceSectors[0];
	SourceSectors[0]->SetRandomSeed(MaxValue);
}

void AMirrorSplineSector::SetSourceSector(ASplineSector* InSourceSector)
{
	SourceSectors.Reset();
	if (InSourceSector)
	{
		SourceSectors.Add(InSourceSector);
		ActiveSourceSector = InSourceSector;
	}
	else
	{
		ActiveSourceSector.Reset();
	}
	RebindSourceDelegates();
	RebuildFromSource();
}

void AMirrorSplineSector::AddSourceSector(ASplineSector* InSourceSector)
{
	if (!InSourceSector) return;

	SourceSectors.AddUnique(InSourceSector);
	if (!ActiveSourceSector.IsValid())
	{
		ActiveSourceSector = InSourceSector;
	}
	RebindSourceDelegates();
	RebuildFromSource();
}

void AMirrorSplineSector::ClearSourceSectors()
{
	SourceSectors.Reset();
	ActiveSourceSector.Reset();
	UnbindSourceDelegates();
	RebuildFromSource();
}

void AMirrorSplineSector::RebuildFromSource()
{
	ASplineSector* TargetSource = ActiveSourceSector.Get();
	if (!TargetSource)
	{
		for (ASplineSector* Source : SourceSectors)
		{
			if (!Source) continue;
			TargetSource = Source;
			ActiveSourceSector = Source;
			break;
		}
	}

	RebuildFromSourceSector(TargetSource);
}

void AMirrorSplineSector::RebuildFromSourceSector(ASplineSector* InSourceSector)
{
	if (!InSourceSector) return;

	ActiveSourceSector = InSourceSector;
	CopySplineFromSource(InSourceSector);
	CopyISMComponentsFromSource(InSourceSector);
}

void AMirrorSplineSector::CopySplineFromSource(const ASplineSector* InSourceSector)
{
	if (!InSourceSector || !RoadSpline || !InSourceSector->RoadSpline) return;

	USplineComponent* SourceSpline = InSourceSector->RoadSpline;
	SplineNum = InSourceSector->SplineNum;
	SplineOffset = InSourceSector->SplineOffset;
	bRandomAtSpawn = InSourceSector->bRandomAtSpawn;
	SplineWidth = InSourceSector->SplineWidth;

	const int32 NumPoints = SourceSpline->GetNumberOfSplinePoints();
	if (NumPoints <= 0)
	{
		Points.Reset();
		RoadSpline->ClearSplinePoints(true);
		return;
	}

	Points.Reset(NumPoints);
	RoadSpline->ClearSplinePoints(false);

	for (int32 i = 0; i < NumPoints; ++i)
	{
		const FVector Point = SourceSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
		const FVector ArriveTangent = SourceSpline->GetArriveTangentAtSplinePoint(i, ESplineCoordinateSpace::Local);
		const FVector LeaveTangent = SourceSpline->GetLeaveTangentAtSplinePoint(i, ESplineCoordinateSpace::Local);
		const FRotator Rotation = SourceSpline->GetRotationAtSplinePoint(i, ESplineCoordinateSpace::Local);
		const FVector Scale = SourceSpline->GetScaleAtSplinePoint(i);
		const ESplinePointType::Type PointType = SourceSpline->GetSplinePointType(i);
		Points.Add(Point);

		RoadSpline->AddSplinePoint(Point, ESplineCoordinateSpace::Local, false);
		RoadSpline->SetTangentsAtSplinePoint(i, ArriveTangent, LeaveTangent, ESplineCoordinateSpace::Local, false);
		RoadSpline->SetRotationAtSplinePoint(i, Rotation, ESplineCoordinateSpace::Local, false);
		RoadSpline->SetScaleAtSplinePoint(i, Scale, false);
		RoadSpline->SetSplinePointType(i, PointType, false);
	}

	RoadSpline->SetClosedLoop(SourceSpline->IsClosedLoop(), false);
	RoadSpline->ReparamStepsPerSegment = SourceSpline->ReparamStepsPerSegment;
	RoadSpline->Duration = SourceSpline->Duration;
	RoadSpline->bStationaryEndpoints = SourceSpline->bStationaryEndpoints;
	RoadSpline->bAllowDiscontinuousSpline = SourceSpline->bAllowDiscontinuousSpline;
	RoadSpline->UpdateSpline();

	const FVector StartLocation = RoadSpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local);
	const FVector EndLocation = RoadSpline->GetLocationAtSplinePoint(NumPoints - 1, ESplineCoordinateSpace::Local);
	Arrow->SetRelativeLocation(FVector(EndLocation.X, EndLocation.Y, StartLocation.Z));
}

void AMirrorSplineSector::RebindSourceDelegates()
{
	UnbindSourceDelegates();

	for (ASplineSector* Source : SourceSectors)
	{
		if (!Source) continue;
		const bool bAlreadyBound = BoundSourceSectors.ContainsByPredicate(
			[Source](const TWeakObjectPtr<ASplineSector>& BoundSource)
			{
				return BoundSource.Get() == Source;
			});
		if (bAlreadyBound) continue;
		Source->OnSplineSectorUpdated.AddUObject(this, &AMirrorSplineSector::HandleSourceSectorUpdated);
		if (Source->PCGComponent)
		{
			Source->PCGComponent->OnPCGGraphGeneratedExternal.AddDynamic(this, &AMirrorSplineSector::HandleSourcePCGGenerated);
		}
		BoundSourceSectors.Add(Source);
	}
}

void AMirrorSplineSector::UnbindSourceDelegates()
{
	for (const TWeakObjectPtr<ASplineSector>& BoundSource : BoundSourceSectors)
	{
		ASplineSector* Source = BoundSource.Get();
		if (!Source) continue;
		Source->OnSplineSectorUpdated.RemoveAll(this);
		if (Source->PCGComponent)
		{
			Source->PCGComponent->OnPCGGraphGeneratedExternal.RemoveDynamic(this, &AMirrorSplineSector::HandleSourcePCGGenerated);
		}
	}
	BoundSourceSectors.Reset();
}

void AMirrorSplineSector::HandleSourceSectorUpdated(ASplineSector* InUpdatedSector)
{
	if (!InUpdatedSector) return;

	// Spline can be mirrored immediately, but PCG output may still be generating.
	CopySplineFromSource(InUpdatedSector);
	ActiveSourceSector = InUpdatedSector;

	if (InUpdatedSector->PCGComponent && InUpdatedSector->PCGComponent->GetGraph())
	{
		return;
	}

	CopyISMComponentsFromSource(InUpdatedSector);
}

void AMirrorSplineSector::HandleSourcePCGGenerated(UPCGComponent* InPCGComponent)
{
	if (!InPCGComponent) return;

	const ASplineSector* SourceSector = Cast<ASplineSector>(InPCGComponent->GetOwner());
	if (!SourceSector) return;

	const bool bTracked = SourceSectors.ContainsByPredicate(
		[SourceSector](const TObjectPtr<ASplineSector>& Source)
		{
			return Source == SourceSector;
		});
	if (!bTracked) return;

	CopyISMComponentsFromSource(SourceSector);
}

void AMirrorSplineSector::ClearCopiedISMComponents()
{
	for (UInstancedStaticMeshComponent* Comp : CopiedISMComponents)
	{
		if (!Comp) continue;
		Comp->DestroyComponent();
	}
	CopiedISMComponents.Empty();
}

void AMirrorSplineSector::CopyISMComponentsFromSource(const ASplineSector* InSourceSector)
{
	ClearCopiedISMComponents();
	if (!InSourceSector) return;

	TArray<UInstancedStaticMeshComponent*> SourceISMs;
	InSourceSector->GetComponents<UInstancedStaticMeshComponent>(SourceISMs);

	for (UInstancedStaticMeshComponent* SourceISM : SourceISMs)
	{
		if (!SourceISM) continue;
		if (!SourceISM->GetStaticMesh()) continue;

		const int32 InstanceCount = SourceISM->GetInstanceCount();
		if (InstanceCount <= 0) continue;

		UInstancedStaticMeshComponent* NewISM = nullptr;
		if (SourceISM->IsA(UHierarchicalInstancedStaticMeshComponent::StaticClass()))
		{
			NewISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
		}
		else
		{
			NewISM = NewObject<UInstancedStaticMeshComponent>(this);
		}
		if (!NewISM) continue;

		NewISM->SetupAttachment(GetRootComponent());
		NewISM->SetRelativeTransform(SourceISM->GetRelativeTransform());
		NewISM->SetStaticMesh(SourceISM->GetStaticMesh());
		NewISM->SetMobility(SourceISM->Mobility);
		NewISM->SetCollisionEnabled(SourceISM->GetCollisionEnabled());
		NewISM->SetCollisionObjectType(SourceISM->GetCollisionObjectType());
		NewISM->SetCollisionResponseToChannels(SourceISM->GetCollisionResponseToChannels());
		NewISM->SetGenerateOverlapEvents(SourceISM->GetGenerateOverlapEvents());
		NewISM->SetCastShadow(SourceISM->CastShadow);
		NewISM->SetVisibility(SourceISM->IsVisible(), true);

		const int32 MaterialCount = SourceISM->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
		{
			if (UMaterialInterface* Material = SourceISM->GetMaterial(MaterialIndex))
			{
				NewISM->SetMaterial(MaterialIndex, Material);
			}
		}

		NewISM->RegisterComponent();

		for (int32 InstanceIndex = 0; InstanceIndex < InstanceCount; ++InstanceIndex)
		{
			FTransform InstanceTransform;
			if (SourceISM->GetInstanceTransform(InstanceIndex, InstanceTransform, false))
			{
				NewISM->AddInstance(InstanceTransform);
			}
		}

		CopiedISMComponents.Add(NewISM);
	}
}
