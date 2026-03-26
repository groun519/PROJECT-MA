// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widget/Destination/DestinationNodeTypes.h"
#include "DestinationNode.generated.h"

class UButton;
class UImage;
class UMaterialInstanceDynamic;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDestinationNodeClicked, FName);

UCLASS()
class P_MA_API UDestinationNode : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void ApplyNodeContext(const FDestinationNodeData& InNodeData, const FVector2D& InSeed, float InOffsetStr);
	const FDestinationNodeData& GetNodeData() const { return NodeData; }
	FOnDestinationNodeClicked& OnDestinationNodeClicked() { return DestinationNodeClicked; }

	/** Visual Material **/
	void SetVisualTranslation(const FVector2D& InTranslation);

	/** Link Anchors **/
	FVector2D GetDestinationTopAnchor(const FGeometry& RootGeometry) const;
	FVector2D GetDestinationBottomAnchor(const FGeometry& RootGeometry) const;

protected:
	/** Visual Material **/
	void RefreshVisualImageMaterialParams();
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> DestinationVisualImage;
	FVector2D MaterialSeed = FVector2D::ZeroVector;
	float MaterialOffsetStr = 0.f;

	/** Button **/
	void RefreshFromNodeData();
	void RefreshButtonState();
	void RefreshPreviewState();
	UFUNCTION()
	void HandleDestinationButtonClicked();
	UFUNCTION()
	void HandleDestinationButtonHovered();
	UFUNCTION()
	void HandleDestinationButtonUnhovered();
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DestinationButton;
	bool bIsButtonHovered = false;
	UPROPERTY()
	FDestinationNodeData NodeData;
	FOnDestinationNodeClicked DestinationNodeClicked;
};
