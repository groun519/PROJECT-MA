// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ListView.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MAAbilityListView.generated.h"

class UGameplayAbility;

/**
 * [추가] 리스트 뷰의 각 슬롯에 전달될 데이터 객체입니다.
 * 슬롯의 키 할당 정보(InputID)와 현재 장착된 스킬 정보(AbilityClass)를 담습니다.
 */
UCLASS()
class UMAAbilitySlotDataObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	EMAAbilityInputID InputID;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	TSubclassOf<UGameplayAbility> AbilityClass;
};

/**
 * 어빌리티 슬롯 리스트 뷰
 */
UCLASS()
class UMAAbilityListView : public UListView
{
	GENERATED_BODY()

public:
	// 스킬 맵을 받아 3개의 슬롯(데이터)을 생성하여 리스트에 채웁니다.
	void ConfigureAbilities(const TMap<EMAAbilityInputID, TSubclassOf<UGameplayAbility>>& Abilities);

private:
	// [삭제] AbilityGaugeGenerated 함수는 NativeOnListItemObjectSet에서 처리하므로 더 이상 델리게이트 바인딩이 필요 없습니다.
	// void AbilityGaugeGenerated(UUserWidget& Widget); 
};