// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadoutIconButtonWidget.generated.h"

class UButton;
class UTexture2D;
class UMaterialInterface;
class UImage;

UCLASS()
class P_MA_API ULoadoutIconButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> IconButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> EquippedBorder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout|Icon")
	TSoftObjectPtr<UTexture2D> IconTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout|Icon")
	TSoftObjectPtr<UMaterialInterface> IconMaterial;

	void SetSelected(bool bInSelected);

protected:
	virtual void NativeConstruct() override;
	virtual void OnButtonClicked();

private:
	UFUNCTION()
	void HandleButtonClicked();

	void ApplyButtonIcon(UObject* ResourceObject, const FVector2D& ImageSize);
	void ApplyBaseStyle();

	bool bSelected = false;
	FButtonStyle BaseStyle;
};
