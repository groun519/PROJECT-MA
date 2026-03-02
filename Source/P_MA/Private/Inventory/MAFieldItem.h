// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Inventory/MAItemTypes.h" 
#include "MAFieldItem.generated.h"

class UStaticMeshComponent;
class UWidgetComponent;
class UDataTable;

/**
 * 필드에 드롭된 아이템 액터
 */
UCLASS()
class P_MA_API AMAFieldItem : public AActor
{
	GENERATED_BODY()
	
public:	
	AMAFieldItem();

protected:
	virtual void BeginPlay() override;

	// 마우스 감지 이벤트 (툴팁 표시용)
	UFUNCTION()
	void OnBeginMouseOver(AActor* TouchedActor);

	UFUNCTION()
	void OnEndMouseOver(AActor* TouchedActor);
	
	// virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "MA|Item")
	FName ItemRowName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MA|Item")
	class UDataTable* ItemDataTable;

	void SetToolTipVisible(bool bVisible);

	class UWidgetComponent* GetToolTipWidgetComp() const { return ToolTipWidgetComp; }

private:
	UPROPERTY(VisibleAnywhere, Category = "MA|Component")
	UStaticMeshComponent* MeshComp;
	
	UPROPERTY(VisibleAnywhere, Category = "MA|Component")
	UWidgetComponent* ToolTipWidgetComp;

	UPROPERTY(VisibleAnywhere, Category = "MA|Component")
	class UWidgetComponent* InteractWidgetComp;
};