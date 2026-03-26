// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DestinationNodeTypes.generated.h"

UENUM(BlueprintType)
enum class EDestinationNodeLinkState : uint8
{
	Candidate,
	Visited,
};

USTRUCT(BlueprintType)
struct FDestinationNodeLinkData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destination")
	FName FromNodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destination")
	FName ToNodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destination")
	EDestinationNodeLinkState LinkState = EDestinationNodeLinkState::Candidate;
};

USTRUCT(BlueprintType)
struct FDestinationLinkStyleParams
{
	GENERATED_BODY()

	/** Link Style **/
	UPROPERTY(EditAnywhere, Category = "Links")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "Links", meta = (ClampMin = "0.1"))
	float Thickness = 4.f;

	UPROPERTY(EditAnywhere, Category = "Links", meta = (ClampMin = "1.0", ClampMax = "200.0"))
	float DashLength = 24.f;

	UPROPERTY(EditAnywhere, Category = "Links", meta = (ClampMin = "1.0", ClampMax = "200.0"))
	float DashGapLength = 14.f;

	UPROPERTY(EditAnywhere, Category = "Links", meta = (ClampMin = "0.0"))
	float CurveHandleOffset = 80.f;

	UPROPERTY(EditAnywhere, Category = "Links", meta = (ClampMin = "2", ClampMax = "64"))
	int32 CurveSegments = 20;
};

USTRUCT(BlueprintType)
struct FDestinationBackgroundMaterialParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Background")
	FVector2D Grid2d = FVector2D(5.f, 5.f);

	UPROPERTY(EditAnywhere, Category = "Background")
	FVector2D Seed = FVector2D(1.f, 1.7f);

	UPROPERTY(EditAnywhere, Category = "Background")
	float OffsetStr = 0.18f;

	UPROPERTY(EditAnywhere, Category = "Background")
	FVector2D GlobalOffset = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Background")
	FLinearColor EnvColor_1 = FLinearColor(0.15f, 0.55f, 0.20f, 1.f);

	UPROPERTY(EditAnywhere, Category = "Background")
	FLinearColor EnvColor_2 = FLinearColor(0.70f, 0.70f, 0.70f, 1.f);

	UPROPERTY(EditAnywhere, Category = "Background")
	FLinearColor EnvColor_3 = FLinearColor(0.87f, 0.63f, 0.16f, 1.f);
};

UENUM(BlueprintType)
enum class EDestinationNodeState : uint8
{
	Current,
	Candidate,
	Visited,
	Unselected,
};

USTRUCT(BlueprintType)
struct FDestinationNodeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destination")
	FName NodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destination")
	FIntPoint GridCoord = FIntPoint(INDEX_NONE, INDEX_NONE);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destination")
	FIntPoint LogicalCoord = FIntPoint(INDEX_NONE, INDEX_NONE);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destination")
	FVector2D NodeGlobalOffset = FVector2D::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destination")
	FVector2D NodeSeedOffset = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destination")
	EDestinationNodeState NodeState = EDestinationNodeState::Current;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destination")
	bool bActiveNode = true;
};
