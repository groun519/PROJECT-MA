// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "Blueprint/UserWidget.h"
#include "Widget/Destination/DestinationNodeTypes.h"
#include "DestinationNodeRoot.generated.h"

class UCanvasPanel;
class UDestinationNode;
class UImage;
class UMaterialInstanceDynamic;

UCLASS()
class P_MA_API UDestinationNodeRoot : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	UFUNCTION(BlueprintCallable, Category = "Destination|Window")
	void RebuildWindowNodes();

	UFUNCTION(BlueprintCallable, Category = "Destination|Window")
	void SetWindowOffsetY(int32 InOffsetY);

	UFUNCTION(BlueprintCallable, Category = "Destination|Window")
	void ShiftWindowOffsetY(int32 DeltaY);

	UFUNCTION(BlueprintCallable, Category = "Destination|Selection")
	void SelectNodeById(FName NodeId);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> NodeCanvas;

	UPROPERTY(EditDefaultsOnly, Category = "Destination")
	TSubclassOf<UDestinationNode> NodeWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Destination|Layout")
	FVector2D NodeWidgetSizeRatio = FVector2D(3.f / 7.f, 3.f / 7.f);

	UPROPERTY(EditAnywhere, Category = "Destination|Style")
	FDestinationLinkStyleParams LinkStyle;

	UPROPERTY(EditAnywhere, Category = "Destination|Input", meta = (ClampMin = "1"))
	int32 MouseWheelStepRows = 1;

private:
	/** Background Material **/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BackgroundImage;
	UPROPERTY(EditAnywhere, Category = "Destination|Background")
	FDestinationBackgroundMaterialParams BackgroundMaterialParams;
	void RefreshBackgroundMaterialParams();
	bool TryGetBackgroundCanvasRect(FVector2D& OutTopLeft, FVector2D& OutSize) const;
	FVector2D GetBaseGridNodeCenter(const FIntPoint& GridCoord, const FVector2D& BackgroundTopLeft, const FVector2D& BackgroundSize) const;
	FVector2D GetGridNodeCenter(const FDestinationNodeData& NodeData, const FVector2D& BackgroundTopLeft, const FVector2D& BackgroundSize) const;
	FVector2D LastBackgroundTopLeft = FVector2D::ZeroVector;
	FVector2D LastBackgroundSize = FVector2D::ZeroVector;
	bool bHasBackgroundRectCache = false;

	/** Links **/
	void DrawConnectionCurve(FPaintContext& PaintContext, const FVector2D& Start, const FVector2D& End, bool bSolidLine) const;
	void RebuildNodeWidgets();
	void ScheduleDeferredNodePositionRefresh();
	void HandleDeferredNodePositionRefresh();
	void RefreshNodeWidgetPositionsIfNeeded();
	void RefreshNodeWidgetPositions();
	void EnsureProgressStateInitialized();

	UPROPERTY()
	TArray<FDestinationNodeData> Nodes;

	UPROPERTY()
	TArray<FDestinationNodeLinkData> Links;

	UPROPERTY()
	TMap<FName, TObjectPtr<UDestinationNode>> NodeWidgetMap;

	UPROPERTY()
	TArray<TObjectPtr<UDestinationNode>> NodeWidgetPool;

	UPROPERTY()
	TArray<int32> SelectedColumnByRow;

	bool bDeferredNodePositionRefreshRequested = false;
};
