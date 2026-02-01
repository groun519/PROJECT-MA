// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MACharacter.h"
#include "Player/MAPlayerCharacter.h"
#include "Core.generated.h"

class UInteractComponent;
class UMaterialInstanceDynamic;
class USkeletalMeshComponent;

USTRUCT(BlueprintType)
struct FCoreMoveSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="MA|Move")
	bool bUseBobMove = true;

	UPROPERTY(EditAnywhere, Category="MA|Move")
	float BobAmplitude = 10.f;

	UPROPERTY(EditAnywhere, Category="MA|Move")
	float BobSpeed = 1.0f;

	UPROPERTY(EditAnywhere, Category="MA|Move")
	bool bUseSpin = true;

	UPROPERTY(EditAnywhere, Category="MA|Move")
	float SpinYawSpeed = 30.f;
};

UCLASS()
class P_MA_API ACore : public AMACharacter
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	ACore();

	/** Interact **/
	UPROPERTY(VisibleAnywhere, Category="MA|Interact") 
	TObjectPtr<UInteractComponent> InteractComp;

	/** Mesh **/
	UPROPERTY(VisibleAnywhere, Category="MA|Mesh")
	TObjectPtr<USkeletalMeshComponent> OuterCoreMesh;

	/** Color **/
	UPROPERTY(EditAnywhere, Category="MA|Material")
	FName ColorParamName = "BaseColor";

	UPROPERTY(EditAnywhere, Category="MA|Material")
	FLinearColor BaseColor = FLinearColor::Blue;

	UPROPERTY(EditAnywhere, Category="MA|Material")
	FLinearColor BattleColor = FLinearColor::Red;

	/** Move Setting **/
	UPROPERTY(EditAnywhere, Category="MA|Move")
	FCoreMoveSettings MoveSetting;

	void ApplyBattleColor(bool bInBattle);

private:
	void HandleInteract(AMAPlayerCharacter* Interactor);
	void ApplyCurrentColor();

	/** Color Lerp member **/
	UPROPERTY(EditAnywhere, Category="MA|Material")
	float ColorInterpDuration = 0.6f;

	FLinearColor CurrentColor = FLinearColor::White;
	FLinearColor StartColor = FLinearColor::White;
	FLinearColor TargetColor = FLinearColor::White;
	float ColorInterpElapsed = 0.f;
	bool bColorInterpActive = false;

	/** Move and Spin member **/
	float BaseRelativeZ = 0.f;
	FRotator BaseRelativeRotation = FRotator::ZeroRotator;
	float BobTime = 0.f;
	float SpinYaw = 0.f;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;
};
