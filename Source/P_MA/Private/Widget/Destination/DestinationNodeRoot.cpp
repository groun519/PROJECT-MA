// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Destination/DestinationNodeRoot.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "Widget/Destination/DestinationNode.h"

namespace
{
	uint32 HashUint(uint32 X)
	{
		X ^= X >> 16;
		X *= 0x7feb352dU;
		X ^= X >> 15;
		X *= 0x846ca68bU;
		X ^= X >> 16;
		return X;
	}

	uint32 HashCombine(uint32 A, uint32 B)
	{
		return HashUint(A ^ (B + 0x9e3779b9U + (A << 6) + (A >> 2)));
	}

	uint32 Hash3(uint32 A, uint32 B, uint32 C)
	{
		return HashCombine(HashCombine(A, B), C);
	}

	float UintToUnitFloat(uint32 X)
	{
		return static_cast<float>(X) * (1.0f / 4294967295.0f);
	}

	FIntPoint CellToIntPoint(const FVector2D& P)
	{
		return FIntPoint(FMath::FloorToInt(P.X), FMath::FloorToInt(P.Y));
	}

	uint32 SeedToUint(const FVector2D& Seed)
	{
		return static_cast<uint32>(FMath::FloorToInt(FMath::Abs(Seed.X) * 10000.0f + 0.5f));
	}

	FVector2D Hash22(const FVector2D& P, uint32 SeedInt)
	{
		const FIntPoint IP = CellToIntPoint(P);
		const uint32 H1 = Hash3(static_cast<uint32>(IP.X), static_cast<uint32>(IP.Y), SeedInt ^ 0x68bc21ebU);
		const uint32 H2 = Hash3(static_cast<uint32>(IP.X), static_cast<uint32>(IP.Y), SeedInt ^ 0x02e5be93U);
		return FVector2D(UintToUnitFloat(H1), UintToUnitFloat(H2));
	}

	FVector2D GetMaterialCell(const FIntPoint& GridCoord)
	{
		return FVector2D(static_cast<float>(GridCoord.X + 1), static_cast<float>(GridCoord.Y + 1));
	}

	FName BuildNodeId(const FIntPoint& LogicalCoord)
	{
		return FName(*FString::Printf(TEXT("Node_%d_%d"), LogicalCoord.X, LogicalCoord.Y));
	}
}

void UDestinationNodeRoot::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureProgressStateInitialized();
	RebuildWindowNodes();
}

FReply UDestinationNodeRoot::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const float WheelDelta = InMouseEvent.GetWheelDelta();
	if (FMath::IsNearlyZero(WheelDelta)) return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
	const int32 Direction = (WheelDelta < 0.f) ? 1 : -1;
	ShiftWindowOffsetY(Direction * FMath::Max(1, MouseWheelStepRows));
	return FReply::Handled();
}

int32 UDestinationNodeRoot::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	FPaintContext PaintContext(AllottedGeometry, MyCullingRect, OutDrawElements,
		LayerId + 1, InWidgetStyle, bParentEnabled);

	for (const FDestinationNodeLinkData& LinkData : Links)
	{
		const TObjectPtr<UDestinationNode>* ParentWidgetPtr = NodeWidgetMap.Find(LinkData.FromNodeId);
		const TObjectPtr<UDestinationNode>* ChildWidgetPtr = NodeWidgetMap.Find(LinkData.ToNodeId);
		if (!ParentWidgetPtr || !ChildWidgetPtr || !ParentWidgetPtr->Get() || !ChildWidgetPtr->Get()) continue;

		const FVector2D ParentCenter = ParentWidgetPtr->Get()->GetDestinationTopAnchor(AllottedGeometry);
		const FVector2D ChildCenter = ChildWidgetPtr->Get()->GetDestinationBottomAnchor(AllottedGeometry);

		DrawConnectionCurve(PaintContext, ParentCenter, ChildCenter, LinkData.LinkState == EDestinationNodeLinkState::Visited);
	}

	return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements,
		LayerId + 2, InWidgetStyle, bParentEnabled);
}

void UDestinationNodeRoot::RefreshBackgroundMaterialParams() {
	UMaterialInstanceDynamic* const BackgroundMID = BackgroundImage->GetDynamicMaterial();
	if (!BackgroundMID) return;

	BackgroundMID->SetVectorParameterValue(TEXT("Grid2d"), FLinearColor(BackgroundMaterialParams.Grid2d.X, BackgroundMaterialParams.Grid2d.Y, 0.f, 0.f));
	BackgroundMID->SetVectorParameterValue(TEXT("Seed"), FLinearColor(BackgroundMaterialParams.Seed.X, BackgroundMaterialParams.Seed.Y, 0.f, 0.f));
	BackgroundMID->SetScalarParameterValue(TEXT("OffsetStr"), BackgroundMaterialParams.OffsetStr);
	BackgroundMID->SetVectorParameterValue(TEXT("GlobalOffset"), FLinearColor(-BackgroundMaterialParams.GlobalOffset.X, -BackgroundMaterialParams.GlobalOffset.Y, 0.f, 0.f));
	BackgroundMID->SetVectorParameterValue(TEXT("EnvColor_1"), BackgroundMaterialParams.EnvColor_1);
	BackgroundMID->SetVectorParameterValue(TEXT("EnvColor_2"), BackgroundMaterialParams.EnvColor_2);
	BackgroundMID->SetVectorParameterValue(TEXT("EnvColor_3"), BackgroundMaterialParams.EnvColor_3);

	const int32 InnerX = FMath::Max(1, FMath::RoundToInt(BackgroundMaterialParams.Grid2d.X));
	uint32 RBits = 0;
	uint32 GBits = 0;
	uint32 BBits = 0;
	for (const FDestinationNodeData& NodeData : Nodes)
	{
		if (!NodeData.bActiveNode) continue;

		const int32 CellIndex = (NodeData.GridCoord.Y * InnerX) + NodeData.GridCoord.X;
		if (CellIndex <= 9) RBits |= (1u << CellIndex);
		else if (CellIndex <= 19) GBits |= (1u << (CellIndex - 10));
		else if (CellIndex <= 29) BBits |= (1u << (CellIndex - 20));
	}

	const FVector SectionMaskBits(static_cast<float>(RBits), static_cast<float>(GBits), static_cast<float>(BBits));
	BackgroundMID->SetVectorParameterValue(TEXT("SectionMaskBits"), FLinearColor(SectionMaskBits.X, SectionMaskBits.Y, SectionMaskBits.Z, 0.f));
}

void UDestinationNodeRoot::DrawConnectionCurve(FPaintContext& PaintContext, const FVector2D& Start, const FVector2D& End, bool bSolidLine) const
{
	auto BezierPoint = [](const FVector2D& P0, const FVector2D& P1, const FVector2D& P2, const FVector2D& P3, float T)
	{
		const float InvT = 1.f - T;
		return
			(InvT * InvT * InvT * P0) +
			(3.f * InvT * InvT * T * P1) +
			(3.f * InvT * T * T * P2) +
			(T * T * T * P3);
	};

	const int32 CurveSegments = FMath::Max(2, LinkStyle.CurveSegments);
	const float VerticalDistance = FMath::Abs(End.Y - Start.Y);
	const float HandleOffset = FMath::Max(LinkStyle.CurveHandleOffset, VerticalDistance * 0.25f);
	const float DirectionY = (End.Y < Start.Y) ? -1.f : 1.f;
	const FVector2D ControlPointA = Start + FVector2D(0.f, DirectionY * HandleOffset);
	const FVector2D ControlPointB = End - FVector2D(0.f, DirectionY * HandleOffset);

	TArray<FVector2D, TInlineAllocator<33>> CurvePoints;
	CurvePoints.Reserve(CurveSegments + 1);
	for (int32 SegmentIndex = 0; SegmentIndex <= CurveSegments; ++SegmentIndex)
	{
		const float T = static_cast<float>(SegmentIndex) / static_cast<float>(CurveSegments);
		CurvePoints.Add(BezierPoint(Start, ControlPointA, ControlPointB, End, T));
	}

	if (bSolidLine)
	{
		for (int32 Index = 1; Index < CurvePoints.Num(); ++Index)
		{
			UWidgetBlueprintLibrary::DrawLine(
				PaintContext,
				CurvePoints[Index - 1],
				CurvePoints[Index],
				LinkStyle.Color,
				true,
				LinkStyle.Thickness);
		}
		return;
	}

	const float SafeDashLength = FMath::Max(1.f, LinkStyle.DashLength);
	const float SafeDashGapLength = FMath::Max(1.f, LinkStyle.DashGapLength);
	const float DashCycleLength = SafeDashLength + SafeDashGapLength;
	float TraversedCurveLength = 0.f;
	for (int32 Index = 1; Index < CurvePoints.Num(); ++Index)
	{
		const FVector2D SegmentStart = CurvePoints[Index - 1];
		const FVector2D SegmentEnd = CurvePoints[Index];
		const FVector2D SegmentVector = SegmentEnd - SegmentStart;
		const float SegmentLength = SegmentVector.Length();
		if (SegmentLength <= KINDA_SMALL_NUMBER) continue;

		const FVector2D SegmentDirection = SegmentVector / SegmentLength;
		float TraversedSegmentLength = 0.f;
		while (TraversedSegmentLength < SegmentLength)
		{
			const float CyclePosition = FMath::Fmod(TraversedCurveLength, DashCycleLength);
			const bool bInDash = CyclePosition < SafeDashLength;
			const float RemainingInPhase = bInDash ? (SafeDashLength - CyclePosition) : (DashCycleLength - CyclePosition);
			const float StepLength = FMath::Min(RemainingInPhase, SegmentLength - TraversedSegmentLength);

			if (bInDash && StepLength > KINDA_SMALL_NUMBER)
			{
				const FVector2D DashStart = SegmentStart + SegmentDirection * TraversedSegmentLength;
				const FVector2D DashEnd = SegmentStart + SegmentDirection * (TraversedSegmentLength + StepLength);
				UWidgetBlueprintLibrary::DrawLine(PaintContext, DashStart, DashEnd, LinkStyle.Color, true, LinkStyle.Thickness);
			}

			TraversedSegmentLength += StepLength;
			TraversedCurveLength += StepLength;
		}
	}
}


bool UDestinationNodeRoot::TryGetBackgroundCanvasRect(FVector2D& OutTopLeft, FVector2D& OutSize) const
{
	const FGeometry& BackgroundGeometry = BackgroundImage->GetCachedGeometry();
	const FGeometry& CanvasGeometry = NodeCanvas->GetCachedGeometry();
	const FVector2D CachedSize = BackgroundGeometry.GetLocalSize();
	const FVector2D CanvasSize = CanvasGeometry.GetLocalSize();
	if (CachedSize.X <= KINDA_SMALL_NUMBER || CachedSize.Y <= KINDA_SMALL_NUMBER || CanvasSize.X <= KINDA_SMALL_NUMBER || CanvasSize.Y <= KINDA_SMALL_NUMBER)
		return false;
	OutSize = CachedSize;
	OutTopLeft = CanvasGeometry.AbsoluteToLocal(BackgroundGeometry.GetAbsolutePosition());
	return true;
}

FVector2D UDestinationNodeRoot::GetBaseGridNodeCenter(
	const FIntPoint& GridCoord,
	const FVector2D& BackgroundTopLeft,
	const FVector2D& BackgroundSize) const
{
	const int32 GridCountX = FMath::Max(1, FMath::RoundToInt(BackgroundMaterialParams.Grid2d.X));
	const int32 GridCountY = FMath::Max(1, FMath::RoundToInt(BackgroundMaterialParams.Grid2d.Y));
	const FVector2D LocalCell = GetMaterialCell(GridCoord);
	const FVector2D TotalCount(static_cast<float>(GridCountX + 2), static_cast<float>(GridCountY + 2));
	const FVector2D GridPosition = (LocalCell + FVector2D(0.5f, 0.5f)) / TotalCount;
	return BackgroundTopLeft + FVector2D(GridPosition.X * BackgroundSize.X, GridPosition.Y * BackgroundSize.Y);
}

void UDestinationNodeRoot::EnsureProgressStateInitialized()
{
	const int32 GridCountX = FMath::Max(1, FMath::RoundToInt(BackgroundMaterialParams.Grid2d.X));
	const int32 CenterColumn = (GridCountX - 1) / 2;
	if (SelectedColumnByRow.IsEmpty()) SelectedColumnByRow.Add(CenterColumn);
}

FVector2D UDestinationNodeRoot::GetGridNodeCenter(
	const FDestinationNodeData& NodeData,
	const FVector2D& BackgroundTopLeft,
	const FVector2D& BackgroundSize) const
{
	const int32 GridCountX = FMath::Max(1, FMath::RoundToInt(BackgroundMaterialParams.Grid2d.X));
	const int32 GridCountY = FMath::Max(1, FMath::RoundToInt(BackgroundMaterialParams.Grid2d.Y));
	const FVector2D LocalCell = GetMaterialCell(NodeData.GridCoord);
	const FVector2D TotalCount(static_cast<float>(GridCountX + 2), static_cast<float>(GridCountY + 2));

	const FVector2D GridPosition =
		(LocalCell + FVector2D(0.5f, 0.5f) + NodeData.NodeSeedOffset) / TotalCount;

	return BackgroundTopLeft + FVector2D(GridPosition.X * BackgroundSize.X, GridPosition.Y * BackgroundSize.Y);
}

void UDestinationNodeRoot::RebuildWindowNodes()
{
	TArray<FDestinationNodeData> WindowNodes;
	TArray<FDestinationNodeLinkData> VisibleLinks;
	TArray<int32> OutgoingColumns;
	const int32 GridCountX = FMath::Max(1, FMath::RoundToInt(BackgroundMaterialParams.Grid2d.X));
	const int32 GridCountY = FMath::Max(1, FMath::RoundToInt(BackgroundMaterialParams.Grid2d.Y));
	const int32 WindowOffsetY = FMath::Max(0, FMath::RoundToInt(BackgroundMaterialParams.GlobalOffset.Y));
	const int32 WindowEndY = WindowOffsetY + GridCountY - 1;
	const uint32 SeedInt = SeedToUint(BackgroundMaterialParams.Seed);
	EnsureProgressStateInitialized();
	const int32 CurrentRow = SelectedColumnByRow.IsEmpty() ? INDEX_NONE : SelectedColumnByRow.Num() - 1;
	if (CurrentRow == INDEX_NONE) return;

	auto GenerateOutgoingColumns = [GridCountX, SeedInt, &OutgoingColumns](const FIntPoint& LogicalCoord)
	{
		OutgoingColumns.Reset();

		struct FScoredColumn
		{
			int32 Column = 0;
			uint32 Score = 0;
		};

		TArray<FScoredColumn, TInlineAllocator<5>> ScoredColumns;
		for (int32 DeltaX = -1; DeltaX <= 1; ++DeltaX)
		{
			const int32 Column = LogicalCoord.X + DeltaX;
			if (Column < 0 || Column >= GridCountX) continue;

			FScoredColumn& Entry = ScoredColumns.AddDefaulted_GetRef();
			Entry.Column = Column;
			Entry.Score = Hash3(
				static_cast<uint32>(LogicalCoord.X),
				static_cast<uint32>(LogicalCoord.Y),
				Hash3(SeedInt ^ 0x2f6e2b1dU, static_cast<uint32>(Column), 0x13572468U));
		}

		ScoredColumns.Sort([](const FScoredColumn& A, const FScoredColumn& B)
		{
			return (A.Score == B.Score) ? (A.Column < B.Column) : (A.Score < B.Score);
		});

		const int32 DesiredCount = FMath::Min(ScoredColumns.Num(), 2 + static_cast<int32>(Hash3(
			static_cast<uint32>(LogicalCoord.X),
			static_cast<uint32>(LogicalCoord.Y),
			SeedInt ^ 0x7a4d3c29U) % 2u));

		for (int32 Index = 0; Index < DesiredCount; ++Index)
			OutgoingColumns.Add(ScoredColumns[Index].Column);

		OutgoingColumns.Sort();
	};

	TMap<FIntPoint, int32> NodeIndexByLogicalCoord;
	for (int32 LocalY = 0; LocalY < GridCountY; ++LocalY)
	{
		for (int32 X = 0; X < GridCountX; ++X)
		{
			const int32 LogicalY = WindowOffsetY + ((GridCountY - 1) - LocalY);
			const FIntPoint LocalGridCoord(X, LocalY);
			const FIntPoint LogicalCoord(X, LogicalY);
			FDestinationNodeData& NodeData = WindowNodes.AddDefaulted_GetRef();
			NodeData.NodeId = BuildNodeId(LogicalCoord);
			NodeData.GridCoord = LocalGridCoord;
			NodeData.LogicalCoord = LogicalCoord;

			NodeData.NodeState = EDestinationNodeState::Unselected;
			NodeData.bActiveNode = false;

			const FVector2D LocalCell = GetMaterialCell(LocalGridCoord);
			const FVector2D PlacementGlobalCell = LocalCell - BackgroundMaterialParams.GlobalOffset;
			const FVector2D PlacementRand = Hash22(PlacementGlobalCell, SeedInt);
			NodeData.NodeGlobalOffset = FVector2D(
				static_cast<float>(LocalGridCoord.X) - BackgroundMaterialParams.GlobalOffset.X,
				static_cast<float>(LocalGridCoord.Y) - BackgroundMaterialParams.GlobalOffset.Y);
			NodeData.NodeSeedOffset = (PlacementRand * 2.f - FVector2D(1.f, 1.f)) * BackgroundMaterialParams.OffsetStr;
			NodeIndexByLogicalCoord.Add(FIntPoint(X, LogicalY), WindowNodes.Num() - 1);
		}
	}

	auto NodeStateRank = [](EDestinationNodeState NodeState)
	{
		switch (NodeState)
		{
		case EDestinationNodeState::Current: return 3;
		case EDestinationNodeState::Candidate: return 2;
		case EDestinationNodeState::Visited: return 1;
		case EDestinationNodeState::Unselected: return 0;
		default: return -1;
		}
	};
	auto MarkNodeState = [&WindowNodes, &NodeStateRank](int32 NodeIndex, EDestinationNodeState NodeState)
	{
		FDestinationNodeData& NodeData = WindowNodes[NodeIndex];
		NodeData.bActiveNode = true;
		if (NodeStateRank(NodeState) >= NodeStateRank(NodeData.NodeState)) NodeData.NodeState = NodeState;
	};
	const int32 CurrentColumn = SelectedColumnByRow[CurrentRow];

	for (int32 LogicalY = WindowOffsetY; LogicalY <= WindowEndY; ++LogicalY)
	{
		if (!SelectedColumnByRow.IsValidIndex(LogicalY) || LogicalY > CurrentRow) continue;

		const int32 SelectedColumn = SelectedColumnByRow[LogicalY];
		const int32* SelectedNodeIndex = NodeIndexByLogicalCoord.Find(FIntPoint(SelectedColumn, LogicalY));
		if (!SelectedNodeIndex) continue;
		MarkNodeState(*SelectedNodeIndex, LogicalY < CurrentRow ? EDestinationNodeState::Visited : EDestinationNodeState::Current);
	}

	for (int32 LogicalY = WindowOffsetY; LogicalY <= FMath::Min(CurrentRow - 1, WindowEndY - 1); ++LogicalY)
	{
		const int32 SelectedColumn = SelectedColumnByRow[LogicalY];
		const FIntPoint SourceLogicalCoord(SelectedColumn, LogicalY);
		GenerateOutgoingColumns(SourceLogicalCoord);
		const FName SourceNodeId = BuildNodeId(SourceLogicalCoord);

		const bool bHasNextSelectedColumn = SelectedColumnByRow.IsValidIndex(LogicalY + 1);
		const int32 NextSelectedColumn = bHasNextSelectedColumn ? SelectedColumnByRow[LogicalY + 1] : INDEX_NONE;

		for (const int32 CandidateColumn : OutgoingColumns)
		{
			if (bHasNextSelectedColumn && CandidateColumn == NextSelectedColumn) continue;

			const int32* CandidateNodeIndex = NodeIndexByLogicalCoord.Find(FIntPoint(CandidateColumn, LogicalY + 1));
			if (!CandidateNodeIndex) continue;

			FDestinationNodeLinkData& LinkData = VisibleLinks.AddDefaulted_GetRef();
			LinkData.FromNodeId = SourceNodeId;
			LinkData.ToNodeId = WindowNodes[*CandidateNodeIndex].NodeId;
			LinkData.LinkState = EDestinationNodeLinkState::Candidate;
			MarkNodeState(*CandidateNodeIndex, EDestinationNodeState::Unselected);
		}
	}

	for (int32 LogicalY = FMath::Max(WindowOffsetY + 1, 1); LogicalY <= FMath::Min(CurrentRow, WindowEndY); ++LogicalY)
	{
		const int32 PreviousColumn = SelectedColumnByRow[LogicalY - 1];
		const int32 SelectedColumn = SelectedColumnByRow[LogicalY];
		const int32* FromNodeIndex = NodeIndexByLogicalCoord.Find(FIntPoint(PreviousColumn, LogicalY - 1));
		const int32* ToNodeIndex = NodeIndexByLogicalCoord.Find(FIntPoint(SelectedColumn, LogicalY));
		if (!FromNodeIndex || !ToNodeIndex) continue;

		FDestinationNodeLinkData& LinkData = VisibleLinks.AddDefaulted_GetRef();
		LinkData.FromNodeId = WindowNodes[*FromNodeIndex].NodeId;
		LinkData.ToNodeId = WindowNodes[*ToNodeIndex].NodeId;
		LinkData.LinkState = EDestinationNodeLinkState::Visited;
	}

	if (CurrentRow >= WindowOffsetY && CurrentRow <= WindowEndY)
	{
		const int32 NextRow = CurrentRow + 1;
		if (NextRow >= WindowOffsetY && NextRow <= WindowEndY)
		{
			const FIntPoint CurrentLogicalCoord(CurrentColumn, CurrentRow);
			const FName CurrentNodeId = BuildNodeId(CurrentLogicalCoord);
			GenerateOutgoingColumns(CurrentLogicalCoord);
			for (const int32 CandidateColumn : OutgoingColumns)
			{
			const int32* ToNodeIndex = NodeIndexByLogicalCoord.Find(FIntPoint(CandidateColumn, NextRow));
			if (!ToNodeIndex) continue;

			FDestinationNodeLinkData& LinkData = VisibleLinks.AddDefaulted_GetRef();
			LinkData.FromNodeId = CurrentNodeId;
			LinkData.ToNodeId = WindowNodes[*ToNodeIndex].NodeId;
			LinkData.LinkState = EDestinationNodeLinkState::Candidate;
			MarkNodeState(*ToNodeIndex, EDestinationNodeState::Candidate);
		}
	}
}

	Links = MoveTemp(VisibleLinks);
	Nodes = MoveTemp(WindowNodes);
	bHasBackgroundRectCache = false;
	RefreshBackgroundMaterialParams();
	RebuildNodeWidgets();
}

void UDestinationNodeRoot::SetWindowOffsetY(int32 InOffsetY)
{
	BackgroundMaterialParams.GlobalOffset.Y = static_cast<float>(FMath::Max(0, InOffsetY));
	RebuildWindowNodes();
}

void UDestinationNodeRoot::ShiftWindowOffsetY(int32 DeltaY)
{
	SetWindowOffsetY(FMath::RoundToInt(BackgroundMaterialParams.GlobalOffset.Y) + DeltaY);
}

void UDestinationNodeRoot::SelectNodeById(FName NodeId)
{
	if (NodeId.IsNone()) return;
	EnsureProgressStateInitialized();
	const int32 CurrentRow = SelectedColumnByRow.IsEmpty() ? INDEX_NONE : SelectedColumnByRow.Num() - 1;
	if (CurrentRow == INDEX_NONE) return;
	const int32 CurrentColumn = SelectedColumnByRow[CurrentRow];
	const FName CurrentNodeId = BuildNodeId(FIntPoint(CurrentColumn, CurrentRow));
	const FDestinationNodeData* SelectedNodeData = nullptr;
	for (const FDestinationNodeData& NodeData : Nodes)
	{
		if (NodeData.NodeId == NodeId)
		{
			SelectedNodeData = &NodeData;
			break;
		}
	}

	if (!SelectedNodeData || SelectedNodeData->LogicalCoord.Y != CurrentRow + 1) return;

	bool bIsCandidate = false;
	for (const FDestinationNodeLinkData& LinkData : Links)
	{
		if (LinkData.LinkState == EDestinationNodeLinkState::Candidate &&
			LinkData.FromNodeId == CurrentNodeId &&
			LinkData.ToNodeId == NodeId)
		{
			bIsCandidate = true;
			break;
		}
	}

	if (!bIsCandidate) return;

	SelectedColumnByRow.Add(SelectedNodeData->LogicalCoord.X);
	RebuildWindowNodes();
}

void UDestinationNodeRoot::RebuildNodeWidgets()
{
	FVector2D BackgroundTopLeft = FVector2D::ZeroVector;
	FVector2D BackgroundSize = FVector2D::ZeroVector;
	const bool bHasBackgroundRect = TryGetBackgroundCanvasRect(BackgroundTopLeft, BackgroundSize);
	const FVector2D NodeWidgetSize = bHasBackgroundRect
		? FVector2D(BackgroundSize.X * NodeWidgetSizeRatio.X, BackgroundSize.Y * NodeWidgetSizeRatio.Y)
		: FVector2D::ZeroVector;
	const int32 DesiredWidgetCount = Nodes.Num();

	while (NodeWidgetPool.Num() < DesiredWidgetCount)
	{
		UDestinationNode* NodeWidget = NodeWidgetClass ? CreateWidget<UDestinationNode>(this, NodeWidgetClass) : nullptr;
		if (!NodeWidget) break;

		NodeWidget->OnDestinationNodeClicked().AddUObject(this, &UDestinationNodeRoot::SelectNodeById);
		NodeCanvas->AddChild(NodeWidget);
		NodeWidgetPool.Add(NodeWidget);

		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(NodeWidget->Slot);
		if (!CanvasSlot) continue;
		CanvasSlot->SetAutoSize(false);
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetPosition(FVector2D::ZeroVector);
		CanvasSlot->SetSize(NodeWidgetSize);
	}

	if (Nodes.IsEmpty())
	{
		NodeWidgetMap.Reset();
		for (const TObjectPtr<UDestinationNode>& NodeWidget : NodeWidgetPool)
		{
			if (!NodeWidget) continue;
			NodeWidget->SetVisualTranslation(FVector2D::ZeroVector);
			NodeWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	const TMap<FName, TObjectPtr<UDestinationNode>> PreviousNodeWidgetMap = NodeWidgetMap;
	NodeWidgetMap.Reset();
	TSet<UDestinationNode*> UsedWidgets;

	for (const FDestinationNodeData& NodeData : Nodes)
	{
		if (!NodeData.bActiveNode) continue;
		const TObjectPtr<UDestinationNode>* ExistingWidget = PreviousNodeWidgetMap.Find(NodeData.NodeId);
		if (!ExistingWidget || !ExistingWidget->Get()) continue;

		UDestinationNode* NodeWidget = ExistingWidget->Get();
		UsedWidgets.Add(NodeWidget);
		NodeWidgetMap.Add(NodeData.NodeId, NodeWidget);
		NodeWidget->ApplyNodeContext(NodeData, BackgroundMaterialParams.Seed, BackgroundMaterialParams.OffsetStr);
	}

	int32 PoolIndex = 0;
	for (const FDestinationNodeData& NodeData : Nodes)
	{
		if (!NodeData.bActiveNode || NodeWidgetMap.Contains(NodeData.NodeId)) continue;
		while (PoolIndex < NodeWidgetPool.Num() && UsedWidgets.Contains(NodeWidgetPool[PoolIndex].Get()))
			++PoolIndex;
		if (PoolIndex >= NodeWidgetPool.Num()) break;

		UDestinationNode* NodeWidget = NodeWidgetPool[PoolIndex++].Get();
		if (!NodeWidget) continue;

		UsedWidgets.Add(NodeWidget);
		NodeWidgetMap.Add(NodeData.NodeId, NodeWidget);
		NodeWidget->ApplyNodeContext(NodeData, BackgroundMaterialParams.Seed, BackgroundMaterialParams.OffsetStr);
	}

	for (const TObjectPtr<UDestinationNode>& NodeWidget : NodeWidgetPool)
	{
		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(NodeWidget->Slot);
		if (!CanvasSlot) continue;
		CanvasSlot->SetSize(NodeWidgetSize);
		if (UsedWidgets.Contains(NodeWidget.Get())) continue;
		NodeWidget->SetVisualTranslation(FVector2D::ZeroVector);
		NodeWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	RefreshNodeWidgetPositions();
	ScheduleDeferredNodePositionRefresh();
}

void UDestinationNodeRoot::ScheduleDeferredNodePositionRefresh()
{
	if (bDeferredNodePositionRefreshRequested || !GetWorld()) return;
	bDeferredNodePositionRefreshRequested = true;
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UDestinationNodeRoot::HandleDeferredNodePositionRefresh);
}

void UDestinationNodeRoot::HandleDeferredNodePositionRefresh()
{
	bDeferredNodePositionRefreshRequested = false;
	RefreshNodeWidgetPositionsIfNeeded();
	if (!bHasBackgroundRectCache && !Nodes.IsEmpty()) ScheduleDeferredNodePositionRefresh();
}

void UDestinationNodeRoot::RefreshNodeWidgetPositionsIfNeeded()
{
	FVector2D BackgroundTopLeft = FVector2D::ZeroVector;
	FVector2D BackgroundSize = FVector2D::ZeroVector;
	if (!TryGetBackgroundCanvasRect(BackgroundTopLeft, BackgroundSize)) return;

	const bool bRectChanged =
		!bHasBackgroundRectCache ||
		!BackgroundTopLeft.Equals(LastBackgroundTopLeft, 0.5f) ||
		!BackgroundSize.Equals(LastBackgroundSize, 0.5f);
	if (!bRectChanged) return;

	LastBackgroundTopLeft = BackgroundTopLeft;
	LastBackgroundSize = BackgroundSize;
	bHasBackgroundRectCache = true;
	RefreshNodeWidgetPositions();
}

void UDestinationNodeRoot::RefreshNodeWidgetPositions()
{
	if (Nodes.IsEmpty()) return;

	FVector2D BackgroundTopLeft = FVector2D::ZeroVector;
	FVector2D BackgroundSize = FVector2D::ZeroVector;
	if (!TryGetBackgroundCanvasRect(BackgroundTopLeft, BackgroundSize)) return;
	const FVector2D NodeWidgetSize = FVector2D(BackgroundSize.X * NodeWidgetSizeRatio.X, BackgroundSize.Y * NodeWidgetSizeRatio.Y);

	for (const FDestinationNodeData& NodeData : Nodes)
	{
		if (!NodeData.bActiveNode || NodeData.GridCoord.X == INDEX_NONE || NodeData.GridCoord.Y == INDEX_NONE) continue;

		const TObjectPtr<UDestinationNode>* NodeWidgetPtr = NodeWidgetMap.Find(NodeData.NodeId);
		if (!NodeWidgetPtr || !NodeWidgetPtr->Get()) continue;
		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(NodeWidgetPtr->Get()->Slot);
		if (!CanvasSlot) continue;

		CanvasSlot->SetSize(NodeWidgetSize);
		const FVector2D BaseCenter = GetBaseGridNodeCenter(NodeData.GridCoord, BackgroundTopLeft, BackgroundSize);
		const FVector2D SeedCenter = GetGridNodeCenter(NodeData, BackgroundTopLeft, BackgroundSize);
		CanvasSlot->SetPosition(SeedCenter);
		NodeWidgetPtr->Get()->SetVisualTranslation(BaseCenter - SeedCenter);
	}
}
