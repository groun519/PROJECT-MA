// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Destination/DestinationNode.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UDestinationNode::NativeConstruct()
{
	Super::NativeConstruct();

	DestinationButton->OnClicked.RemoveDynamic(this, &UDestinationNode::HandleDestinationButtonClicked);
	DestinationButton->OnClicked.AddDynamic(this, &UDestinationNode::HandleDestinationButtonClicked);
	DestinationButton->OnHovered.RemoveDynamic(this, &UDestinationNode::HandleDestinationButtonHovered);
	DestinationButton->OnHovered.AddDynamic(this, &UDestinationNode::HandleDestinationButtonHovered);
	DestinationButton->OnUnhovered.RemoveDynamic(this, &UDestinationNode::HandleDestinationButtonUnhovered);
	DestinationButton->OnUnhovered.AddDynamic(this, &UDestinationNode::HandleDestinationButtonUnhovered);
        DestinationVisualImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	RefreshVisualImageMaterialParams();
	RefreshFromNodeData();
}

void UDestinationNode::ApplyNodeContext(const FDestinationNodeData& InNodeData, const FVector2D& InSeed, float InOffsetStr)
{
	if (NodeData.NodeId != InNodeData.NodeId) bIsButtonHovered = false;
	NodeData = InNodeData;
	MaterialSeed = InSeed;
	MaterialOffsetStr = InOffsetStr;
	RefreshVisualImageMaterialParams();
	RefreshFromNodeData();
}

void UDestinationNode::SetVisualTranslation(const FVector2D& InTranslation)
{
	DestinationVisualImage->SetRenderTranslation(InTranslation);
}

FVector2D UDestinationNode::GetDestinationTopAnchor(const FGeometry& RootGeometry) const
{
	const FGeometry& Geometry = DestinationButton->GetPaintSpaceGeometry();
	const FVector2D Size = Geometry.GetLocalSize();
	return RootGeometry.AbsoluteToLocal(Geometry.LocalToAbsolute(FVector2D(Size.X * 0.5f, 0.f)));
}

FVector2D UDestinationNode::GetDestinationBottomAnchor(const FGeometry& RootGeometry) const
{
	const FGeometry& Geometry = DestinationButton->GetPaintSpaceGeometry();
	const FVector2D Size = Geometry.GetLocalSize();
	return RootGeometry.AbsoluteToLocal(Geometry.LocalToAbsolute(FVector2D(Size.X * 0.5f, Size.Y)));
}

void UDestinationNode::RefreshVisualImageMaterialParams()
{
	UMaterialInstanceDynamic* const VisualMID = DestinationVisualImage->GetDynamicMaterial();
	if (!VisualMID) return;

	VisualMID->SetVectorParameterValue(TEXT("GlobalOffset"), FLinearColor(NodeData.NodeGlobalOffset.X, NodeData.NodeGlobalOffset.Y, 0.f, 0.f));
	VisualMID->SetVectorParameterValue(TEXT("Seed"), FLinearColor(MaterialSeed.X, MaterialSeed.Y, 0.f, 0.f));
	VisualMID->SetScalarParameterValue(TEXT("OffsetStr"), MaterialOffsetStr);
}

void UDestinationNode::RefreshFromNodeData()
{
	SetVisibility(NodeData.bActiveNode ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	if (!NodeData.bActiveNode) return;

	RefreshButtonState();
	RefreshPreviewState();
}

void UDestinationNode::RefreshButtonState()
{
	const bool bShrinkButton =
		NodeData.NodeState == EDestinationNodeState::Visited ||
		NodeData.NodeState == EDestinationNodeState::Unselected;
	const bool bButtonEnabled = NodeData.NodeState == EDestinationNodeState::Candidate;

	DestinationButton->SetVisibility(bButtonEnabled ? ESlateVisibility::Visible : ESlateVisibility::HitTestInvisible);
	DestinationButton->SetIsEnabled(bButtonEnabled);
	DestinationButton->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	DestinationButton->SetRenderScale(FVector2D(bShrinkButton ? (2.f / 3.f) : 1.f, bShrinkButton ? (2.f / 3.f) : 1.f));
}

void UDestinationNode::RefreshPreviewState()
{
	if (bIsButtonHovered)
	{
		DestinationVisualImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, .75f));
		return;
	}

	if (NodeData.NodeState == EDestinationNodeState::Unselected)
	{
		DestinationVisualImage->SetColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.1f, .75f));
		return;
	}

	DestinationVisualImage->SetColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.f));
}

void UDestinationNode::HandleDestinationButtonClicked()
{
	DestinationNodeClicked.Broadcast(NodeData.NodeId);
}

void UDestinationNode::HandleDestinationButtonHovered()
{
	bIsButtonHovered = true;
	RefreshFromNodeData();
}

void UDestinationNode::HandleDestinationButtonUnhovered()
{
	bIsButtonHovered = false;
	RefreshFromNodeData();
}
